#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <fstream>
#include <iomanip>
#include "../include/event.h" 
#include "../include/dataBaseClient.h" 
#include "../include/ConnectionHandler.h"
#include <string>

// Mutex for thread synchronization
std::mutex mtx;
std::atomic<bool> stopThreads(false);
int recipt = 0;
int id = 0;
DataBaseClient* userMessages = new DataBaseClient();
std::string userName;

// Convert user input to a STOMP frame
std::string convertToStompFrame(const std::string &userInput)
{
    std::vector<std::string> frames;
    // Parse user input and create the appropriate STOMP frame
    std::string frame;
    if (starts_with(userInput, "login"))
    {
        std::string parts = userInput.substr(6); // Remove "login "
        size_t colonPos = parts.find(':');
        //std::string host = parts.substr(0, colonPos);
        size_t spacePos = parts.find(' ', colonPos);
        //std::string port = parts.substr(colonPos + 1, spacePos - colonPos - 1);
        std::string username = parts.substr(spacePos + 1, parts.find(' ', spacePos + 1) - spacePos - 1);
        std::string password = parts.substr(parts.find(' ', spacePos + 1) + 1);
		userName = username;
        frames.push_back("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il"               
                "\nlogin:" + username + "\npasscode:" + password + "\n\n^0"); 
    }
    else if (starts_with(userInput,"join"))
    {
        std::string channel = userInput.substr(5);
        frames.push_back("SUBSCRIBE\ndestination:/" + channel + "\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(recipt) + "\n\n^0"); //????????????????????
        recipt++;
		id++;
    }
    else if (starts_with(userInput, "exit"))
    {
        std::string channel = userInput.substr(5);
        frames.push_back("UNSUBSCRIBE\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(recipt) + "\n\n^0");
        recipt++;
		id++;

    }
    else if (starts_with(userInput, "logout"))
    {
        frames.push_back("DISCONNECT\nreceipt:" + std::to_string(recipt) + "\n\n^0");
        recipt++;
		userMessages->deleteUser(userName);
		userName = "";
    }
    else if (starts_with(userInput, "report")) 
    {
        std::string filePath = userInput.substr(7); // Skip "report "
        frames = jsonToEvent(filePath);
    }

    else if (starts_with(userInput,"summery")) //?????????????????????
    {
        std::istringstream iss(userInput.substr(8)); // Skip "summary "
        std::string channelName, userName, filePath;
        iss >> channelName >> userName >> filePath;
        std::vector<Event> events = userMessages->getEvents(userName, channelName);
        sortEvents(events);
        generateSummary(channelName, userName, filePath, events);
    }

    return frame;
}

// Keyboard thread function
void readFromKeyboard(ConnectionHandler &connectionHandler)
{
    while (!stopThreads)
    {
        std::string userInput;
        std::getline(std::cin, userInput);
        std::string stompFrame = convertToStompFrame(userInput);

        std::lock_guard<std::mutex> lock(mtx);
        // Send the frame to the server
        if (!connectionHandler.sendLine(stompFrame))
        {
            std::cerr << "Failed to send frame to server.\n";
            stopThreads = true;
        }

        if (starts_with(userInput, "logout"))
        {
            stopThreads = true;
        }
    }
}

// Server thread function
void readFromServer(ConnectionHandler &connectionHandler)
{
    while (!stopThreads)
    {
        std::string serverResponse;
        if (!connectionHandler.getLine(serverResponse))
        {
            // std::cerr << ; הודעת ארור
            stopThreads = true;
            break;
        }

        std::lock_guard<std::mutex> lock(mtx);
        // Process server response
        std::cout << "Server: " << serverResponse << std::endl;
        if (starts_with(serverResponse, "RECEIPT"))
        {
            std::cout << "Received acknowledgment from server.\n";
        }
        else if (starts_with(serverResponse, "ERROR"))
        {
            std::cerr << "Error received from server: " << serverResponse << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " host port\n";
        return -1;
    }

    std::string host = argv[1];
    short port = atoi(argv[2]);
    ConnectionHandler connectionHandler(host, port);

    if (!connectionHandler.connect())
    {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    // Start keyboard and server threads
    std::thread keyboardThread(readFromKeyboard, std::ref(connectionHandler));
    std::thread serverThread(readFromServer, std::ref(connectionHandler));

    // Join threads
    keyboardThread.join();
    serverThread.join();
	delete userMessages;
    return 0;
}

void sortEventsByTimeApoch(std::vector<Event> &events)
{
    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b)
              { return a.get_date_time() < b.get_date_time(); });
}

// Comparator for sorting by date/time and lexicographically by name
bool compareByDateAndName(const Event &a, const Event &b)
{
    if (a.get_date_time() == b.get_date_time())
    {
        // Secondary criterion: Lexicographical comparison by event name
        return a.get_name() < b.get_name();
    }
    // Primary criterion: Compare date_time strings lexicographically
    return a.get_date_time() < b.get_date_time();
}

// Sort events by date/time and name
void sortEvents(std::vector<Event> &events)
{
    std::sort(events.begin(), events.end(), compareByDateAndName);
}

std::vector<std::string> jsonToEvent(std::string filepath)
{
    names_and_events parsedData = parseEventsFile(filepath);
    sortEvents(parsedData.events);
    std::vector<std::string> frames; // To store multiple frames for "report"
    // Create SEND frames for each event
    for (const Event &event : parsedData.events)
    {
		userMessages->addReport(userName, event.get_channel_name(), event);
        std::ostringstream sendFrame;
        sendFrame << "SEND\n"
                  << "destination:/" << event.get_channel_name() << "\n\n"
                  << "user:" << event.getEventOwnerUser() << "\n"
                  << "city:" << event.get_city() << "\n"
                  << "event name:" << event.get_name() << "\n"
                  << "date time:" << event.get_date_time() << "\n"
                  << "general information:\n"
                  << "    active:" << event.get_general_information().at("active") << "\n"
                  << "    forces arrival at scene:" << event.get_general_information().at("forces_arrival_at_scene") << "\n"
                  << "description:" << event.get_description() << "\n"
                  << "^0";
        frames.push_back(sendFrame.str());
    }
    return frames;
}
    
    void generateSummary(const std::string &channelName, const std::string &userName,
                         const std::string &filePath, const std::vector<Event> &events)
    {
        std::ofstream outFile(filePath, std::ios::app);
        if (!outFile)
        {
            std::cerr << "Failed to open file: " << filePath << std::endl;
        }
        int totalEvents = 0, activeEvents = 0, forcedArrivals = 0;

        outFile << "Channel: " << channelName << "\n";
        outFile << "Stats:\n";

        for (const Event &event : events)
        {
            if (event.get_channel_name() == channelName && event.getEventOwnerUser() == userName)
            {
                totalEvents++;
                if (event.get_general_information().at("active") == "true")
                    activeEvents++;
                if (event.get_general_information().at("forces_arrival_at_scene") == "true")
                    forcedArrivals++;
            }
        }
        outFile << "Total: " << totalEvents << "\n";
        outFile << "active: " << activeEvents << "\n";
        outFile << "forces arrival at scene: " << forcedArrivals << "\n";

        // Write event reports
        outFile << "Event Reports:\n";
        int reportIndex = 1;

        for (const Event &event : events)
        {
            if (event.get_channel_name() == channelName && event.getEventOwnerUser() == userName)
            {
                // Convert timestamp to readable date
                std::time_t timestamp = event.get_date_time();
                std::tm *tm = std::localtime(&timestamp);
                std::ostringstream dateStream;
                dateStream << std::put_time(tm, "%d/%m/%Y %H:%M");

                // Truncate description
                std::string truncatedDescription = event.get_description();
                if (truncatedDescription.length() > 27)
                {
                    truncatedDescription = truncatedDescription.substr(0, 27) + "...";
                }

                outFile << "Report_" << reportIndex++ << ":\n";
                outFile << "city: " << event.get_city() << "\n";
                outFile << "date time: " << dateStream.str() << "\n";
                outFile << "event name: " << event.get_name() << "\n";
                outFile << "summary: " << truncatedDescription << "\n";
            }
        }
        outFile.close();
        std::cout << "Summary generated in file: " << filePath << std::endl;
    }
    bool starts_with(const std::string &source, const std::string &pre)
    {
        int spacePos = source.find(' ');
        std::string start = source.substr(0, spacePos);
        return start==pre;

    }



