#include "../include/StompProtocol.h"
#include <fstream>
#include <iomanip>
#include <Task.h>

StompProtocol::StompProtocol()
    : recipt(0), id(0), userMessages(DataBaseClient()),
      namesAndPasswords(std::map<std::string, std::string>()),
      idInChannel(std::map<std::string, int>()), userName(""), connectionHandler(nullptr), serverThread(nullptr), mtx()
{
    login.store(false);
    stopThreadsServer.store(false);
    stopThreadsServer.store(false);
}

/**
 * @brief Check if a string starts with a given prefix.
 *
 * @param source
 * @param pre
 * @return true
 * @return false
 */
bool StompProtocol::starts_with(const std::string &source, const std::string &pre)
{
    return source.find(pre) == 0;
}

/**
 * @brief Compare events by date and name.
 *
 * @param a
 * @param b
 * @return true
 * @return false
 */
bool StompProtocol::compareByDateAndName(const Event &a, const Event &b)
{
    if (a.get_date_time() == b.get_date_time())
    {
        // Secondary criterion: Lexicographical comparison by event name
        return a.get_name() < b.get_name();
    }
    // Primary criterion: Compare date_time strings lexicographically
    return a.get_date_time() < b.get_date_time();
}

/**
 * @brief Sort events by date and name.
 *
 * @param events
 */
void StompProtocol::sortEvents(std::vector<Event> &events)
{
    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b)
              {
                  // Sort primarily by date, and secondarily by name
                  if (a.get_date_time() != b.get_date_time())
                  {
                      return a.get_date_time() < b.get_date_time(); // Ascending order by date
                  }
                  return a.get_name() < b.get_name(); // Ascending order by name
              });
}

/**
 * @brief Parse a JSON file containing events.
 *
 * @param filepath
 * @return std::vector<std::string>
 */
std::vector<std::string> StompProtocol::jsonToEvent(std::string filepath)
{
    names_and_events parsedData = parseEventsFile(filepath);
    sortEvents(parsedData.events);
    std::vector<std::string> frames; // To store multiple frames for "report"
    // Create SEND frames for each event
    for (const Event &event : parsedData.events)
    {
        if (idInChannel.find(event.get_channel_name()) == idInChannel.end())
        {
            std::cout << "you are not registered to channel" + event.get_channel_name() << std::endl;
            break;
        }
        else
        {
            std::ostringstream sendFrame;
            sendFrame << "SEND\n"
                      << "destination:/" << event.get_channel_name() << "\n\n"
                      << "user:" << userName << "\n"
                      << "city:" << event.get_city() << "\n"
                      << "event name:" << event.get_name() << "\n"
                      << "date time:" << event.get_date_time() << "\n"
                      << "general information:\n"
                      << "\tactive:" << event.get_general_information().at("active") << "\n"
                      << "\tforces arrival at scene:" << event.get_general_information().at("forces_arrival_at_scene") << "\n"
                      << "description:" << event.get_description() << "\n";
            frames.push_back(sendFrame.str());
        }
    }
    return frames;
}

/**
 * @brief Generate a summary of events.
 *
 * @param channelName
 * @param userName
 * @param filePath
 * @param events
 */
void StompProtocol::generateSummary(const std::string &channelName, const std::string &userName,
                                    const std::string &filePath, const std::vector<Event> &events)
{
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile)
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
    int totalEvents = 0, activeEvents = 0, forcedArrivals = 0;

    outFile << "Channel: " << channelName << "\n";
    if (idInChannel.find(channelName) != idInChannel.end())
    {
        outFile << "Stats:\n";

        for (const Event &event : events)
        {
            totalEvents++;
            if (event.get_general_information().at("active") == "true")
                activeEvents++;
            if (event.get_general_information().at("forces_arrival_at_scene") == "true")
                forcedArrivals++;
        }
        outFile << "Total: " << totalEvents << "\n";
        outFile << "active: " << activeEvents << "\n";
        outFile << "forces arrival at scene: " << forcedArrivals << "\n";

        // Write event reports
        outFile << "Event Reports:\n\n";
        int reportIndex = 1;

        for (const Event &event : events)
        {
            std::time_t timestamp = static_cast<std::time_t>(event.get_date_time());

            // Convert epoch to a tm structure
            std::tm *tm = std::localtime(&timestamp);

            // Format the date and time
            std::ostringstream oss;
            oss << std::put_time(tm, "%d/%m/%Y %H:%M:%S");

            // Truncate description
            std::string truncatedDescription = event.get_description();
            if (truncatedDescription.length() > 27)
            {
                truncatedDescription = truncatedDescription.substr(0, 27) + "...";
            }
            outFile << "Report_" << reportIndex << ":\n";
            reportIndex++;
            outFile << "city: " << event.get_city() << "\n";
            outFile << "date time: " << oss.str() << "\n";
            outFile << "event name: " << event.get_name() << "\n";
            outFile << "summary:" << truncatedDescription << "\n\n";
        }
        outFile.close();
        std::cout << "Summary generated in file: " << filePath << std::endl;
    }
}

/**
 * @brief Read user input from the keyboard. Convert it to STOMP frames and send it to the server.
 *
 */
void StompProtocol::readFromKeyboard()
{
    while (true)
    {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        int len = line.length();

        std::vector<std::string> stompFrames = StompProtocol::convertToStompFrame(line);

        for (std::string &frame : stompFrames)
        {
            std::lock_guard<std::mutex> lock(mtx);
            // Send the frame to the server
            if (!connectionHandler->sendLine(frame))
            {
                std::cerr << "Failed to send frame to server.\n";
                stopThreadsServer = true;
            }

            if (StompProtocol::starts_with(line, "logout"))
            {
                stopThreadsServer = true;
            }
        }
    }
}

/**
 * @brief Convert user input to a STOMP frame.
 *
 * @param userInput
 * @return std::vector<std::string>
 */
std::vector<std::string> StompProtocol::convertToStompFrame(const std::string &userInput)
{
    std::vector<std::string> frames;
    // Parse user input and create the appropriate STOMP frame
    if ((starts_with(userInput, "login")) & (!login))
    {
        std::string parts = userInput.substr(6); // Remove "login "
        size_t colonPos = parts.find(':');
        if (colonPos == std::string::npos)
        {
            std::cout << "port is illegal" << std::endl;
        }
        std::string host = parts.substr(0, colonPos);
        size_t spacePos = parts.find(' ', colonPos);
        if (spacePos == std::string::npos)
        {
            std::cout << "login command needs 3 args: {host:port} {username} {password}" << std::endl;
        }
        std::string port = parts.substr(colonPos + 1, spacePos - colonPos - 1);
        std::string username = parts.substr(spacePos + 1, parts.find(' ', spacePos + 1) - spacePos - 1);
        size_t spacePos2 = parts.find(' ', spacePos + 1);
        std::string password = parts.substr(spacePos2 + 1);
        if (spacePos2 == std::string::npos)
        {
            std::cout << "login command needs 3 args: {host:port} {username} {password}" << std::endl;
        }

        size_t spacePos3 = parts.find(' ', spacePos2 + 1);

        if (spacePos3 != std::string::npos)
        {
            std::cout << "login command needs 3 args: {host:port} {username} {password}" << std::endl;
        }
        userName = username;
        connectionHandler = new ConnectionHandler(host, std::stoi(port));
        if (!connectionHandler->connect())
        {
            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        }
        else
        {
            std::cout << "Connected to " << host << ":" << port << std::endl;
            // Start server thread
            stopThreadsServer = false;
            // Create a Task object and run it in a separate thread

            Task task;
            serverThread = new std::thread(&Task::Run, &task);

            frames.push_back("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il"
                             "\nlogin:" +
                             username + "\npasscode:" + password + "\n\n");
            login.store(true);
        }
    }
    else if (starts_with(userInput, "join"))
    {
        {
            if (!login)
            {
                std::cout << "please login first" << std::endl;
            }
            else
            {
                std::string parts;
                if (userInput.size() >= 5)
                {
                    parts = userInput.substr(5); // Remove "join "
                    size_t spacePos = parts.find(' ');
                    if (spacePos != std::string::npos || parts.size() == 0)
                    {
                        std::cout << "join command needs 1 args: {channel_name}" << std::endl;
                    }
                    else
                    {
                        if (idInChannel.find(parts) != idInChannel.end())
                        {
                            std::cout << "you are already subscribed to channel" + parts << std::endl;
                        }
                        else
                        {
                            frames.push_back("SUBSCRIBE\ndestination:/" + parts + "\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(recipt) + "\n\n");
                            (idInChannel)[parts] = id;
                            receiptToMessage[recipt] = "SUBSCRIBE";
                            recipt++;
                            id++;
                        }
                    }
                }
                else
                {
                    std::cout << "join command needs 1 args: {channel_name}" << std::endl;
                }
            }
        }
    }
    else if (starts_with(userInput, "exit"))
    {
        if (!login)
        {
            std::cout << "please login first" << std::endl;
        }
        else
        {
            std::string parts;
            if (userInput.size() >= 5)
            {
                parts = userInput.substr(5); // Remove "exit "
                size_t spacePos = parts.find(' ');
                if (spacePos != std::string::npos || parts.size() == 0)
                {
                    std::cout << "exit command needs 1 args: {channel_name}" << std::endl;
                }
                else
                {
                    if (idInChannel.find(parts) == idInChannel.end())
                    {
                        std::cout << "you are not subscribed to channel" + parts << std::endl;
                    }
                    else
                    {
                        frames.push_back("UNSUBSCRIBE\nid:" + std::to_string((idInChannel)[parts]) + "\nreceipt:" + std::to_string(recipt) + "\n\n");
                        receiptToMessage[recipt] = "UNSUBSCRIBE";
                        recipt++;
                        idInChannel.erase(parts);
                    }
                }
            }
            else
            {
                std::cout << "exit command needs 1 args: {channel_name}" << std::endl;
            }
        }
    }
    else if (starts_with(userInput, "logout"))
    {
        size_t spacePos = userInput.find(' ');
        if (spacePos != std::string::npos)
        {
            std::cout << "logout command needs 0 args" << std::endl;
        }
        if (!login)
        {
            std::cout << "please login first" << std::endl;
        }
        else
        {
            frames.push_back("DISCONNECT\nreceipt:" + std::to_string(recipt) + "\n\n");
            receiptToMessage[recipt] = "DISCONNECT";
            recipt++;
            userMessages.deleteUser(userName);
            userName = "";
        }
    }
    else if (starts_with(userInput, "report"))
    {
        if (!login)
        {
            std::cout << "please login first" << std::endl;
        }
        else
        {
            std::string filePath;
            if (userInput.size() >= 7)
            {
                filePath = userInput.substr(7); // Skip "report "
                size_t spacePos = filePath.find(' ');
                if (spacePos != std::string::npos || filePath.size() == 0)
                {
                    std::cout << "report command needs 1 args: {channel_name}" << std::endl;
                }
                else
                    frames = jsonToEvent(filePath);
            }
            else
            {
                std::cout << "report command needs 1 args: {channel_name}" << std::endl;
            }
        }
    }

    else if (starts_with(userInput, "summary"))
    {
        std::istringstream iss(userInput); // Skip "summary "
        std::string param;
        std::vector<std::string> params;
        while (iss >> param)
        {
            params.push_back(param);
        }
        if (params.size() != 4)
        {
            std::cout << "summary command needs 3 args: {channel_name} {username} {filepath}" << std::endl;
        }
        else
        {
            std::string searchchannelName = "/" + params[1];
            if (userMessages.getEvents(userName, searchchannelName).size() == 0)
            {
                std::cout << "no reports to summarize" << std::endl;
            }
            else
            {
                std::cout << params[0] << std::endl;
                // Get all events for the user and channel
                std::vector<Event> events = userMessages.getEvents(userName, searchchannelName);
                // Sort events by date and name
                sortEvents(events);
                // Generate summary
                generateSummary(params[1], userName, params[3], events);
            }
        }
    }

    else
    {
        std::cout << "Invalid command" << std::endl;
    }
    return frames;
}
/**
 *
 * @brief Disconnect from the current socket. And prepare for a new connection.
 *
 */

void StompProtocol::disconnectFromCurrentSocket()
{
    stopThreadsServer.store(true);
    connectionHandler->close();
    delete connectionHandler;
    connectionHandler = nullptr;
    login = false;
    userMessages.deleteData();
    idInChannel.clear();
}

const std::atomic<bool> &StompProtocol::getStopThreadsServer()
{
    return stopThreadsServer;
}

ConnectionHandler &StompProtocol::getConnectionHandler()
{
    return *connectionHandler;
}

std::mutex &StompProtocol::getMutex()
{
    return mtx;
}
std::unordered_map<int, std::string> &StompProtocol::getReceiptToMessage()
{
    return receiptToMessage;
}
DataBaseClient &StompProtocol::getUserMessages()
{
    return userMessages;
}
std::string &StompProtocol::getUserName()
{
    return userName;
}
