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
std::atomic<bool> stopThreadsKeyboard(false);
std::atomic<bool> stopThreadsServer(false);

int recipt = 0;
int id = 0;
DataBaseClient *userMessages = new DataBaseClient();
std::string userName;
std::map<std::string, std::string> *namesAndPasswords = new std::map<std::string, std::string>();
std::map<std::string, int> *idInChannel = new std::map<std::string, int>();
bool login = false;

/**
 * @brief Check if a string starts with a given prefix.
 *
 * @param source
 * @param pre
 * @return true
 * @return false
 */
bool starts_with(const std::string &source, const std::string &pre)
{
	int spacePos = source.find(' ');
	std::string start = source.substr(0, spacePos);
	return start == pre;
}

/**
 * @brief Compare events by date and name.
 *
 * @param a
 * @param b
 * @return true
 * @return false
 */
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

/**
 * @brief Sort events by date and name.
 *
 * @param events
 */
void sortEvents(std::vector<Event> &events)
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
std::vector<std::string> jsonToEvent(std::string filepath)
{
	names_and_events parsedData = parseEventsFile(filepath);
	sortEvents(parsedData.events);
	std::vector<std::string> frames; // To store multiple frames for "report"
	// Create SEND frames for each event
	for (const Event &event : parsedData.events)
	{
		if (idInChannel->find(event.get_channel_name()) == idInChannel->end())
		{
			std::cout << "you are not registered to channel" + event.get_channel_name() << std::endl;
		}
		userMessages->addReport(userName, event.get_channel_name(), event);
		std::ostringstream sendFrame;
		sendFrame << "SEND\n"
				  << "destination:/" << event.get_channel_name() << "\n\n"
				  << "user:" << userName << "\n"
				  << "city:" << event.get_city() << "\n"
				  << "event name:" << event.get_name() << "\n"
				  << "date time:" << event.get_date_time() << "\n"
				  << "general information:\n"
				  << "    active:" << event.get_general_information().at("active") << "\n"
				  << "    forces arrival at scene:" << event.get_general_information().at("forces_arrival_at_scene") << "\n"
				  << "description:" << event.get_description() << "\n";
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

// Convert user input to a STOMP frame
std::vector<std::string> convertToStompFrame(const std::string &userInput)
{
	std::vector<std::string> frames;
	// Parse user input and create the appropriate STOMP frame
	if (starts_with(userInput, "login"))
	{
		std::string parts = userInput.substr(6); // Remove "login "
		size_t colonPos = parts.find(':');
		if (colonPos == std::string::npos)
		{
			std::cout << "port are illegal" << std::endl;
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
		if (namesAndPasswords->find(username) == namesAndPasswords->end())
		{
			namesAndPasswords->insert(std::make_pair(username, password));
		}
		else if (password != namesAndPasswords->at(username))
		{
			std::cout << "wrong password" << std::endl;
		}
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
		frames.push_back("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il"
						 "\nlogin:" +
						 username + "\npasscode:" + password + "\n\n");
		login = true;
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
				}
				else
				{
					std::cout << "join command needs 1 args: {channel_name}" << std::endl;
				}

				frames.push_back("SUBSCRIBE\ndestination:/" + parts + "\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(recipt) + "\n\n");
				(*idInChannel)[parts] = id;
				recipt++;
				id++;
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
			}
			else
			{
				std::cout << "exit command needs 1 args: {channel_name}" << std::endl;
			}

			if (idInChannel->find(parts) == idInChannel->end())
			{
				std::cout << "you are not subscribed to channel" + parts << std::endl;
			}
			frames.push_back("UNSUBSCRIBE\nid:" + std::to_string((*idInChannel)[parts]) + "\nreceipt:" + std::to_string(recipt) + "\n\n");
			recipt++;
			idInChannel->erase(parts);
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
			recipt++;
			userMessages->deleteUser(userName);
			userName = "";
			login = false;
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
					std::cout << "exit command needs 1 args: {channel_name}" << std::endl;
				}
			}
			else
			{
				std::cout << "exit command needs 1 args: {channel_name}" << std::endl;
			}
			frames = jsonToEvent(filePath);
		}
	}

	else if (starts_with(userInput, "summary"))
	{
		std::istringstream iss(userInput.substr(8)); // Skip "summary "
		std::string channelName, userName, filePath;
		iss >> channelName >> userName >> filePath;
		if (userMessages->getEvents(userName, channelName).size() == 0)
		{
			std::cout << "no reports to summarize" << std::endl;
		}
		std::cout << channelName << std::endl;
		std::vector<Event> events = userMessages->getEvents(userName, channelName);
		for (const Event &event : events)
		{
			std::cout << event.get_name() << std::endl;
			std::cout << event.get_date_time() << "\n"
					  << std::endl;
		}
		sortEvents(events);
		for (const Event &event : events)
		{
			std::cout << event.get_name() << std::endl;
			std::cout << event.get_date_time() << "\n"
					  << std::endl;
		}

		generateSummary(channelName, userName, filePath, events);
	}

	return frames;
}

// Keyboard thread function
void readFromKeyboard(ConnectionHandler &connectionHandler)
{
	while (!stopThreadsKeyboard)
	{
		const short bufsize = 1024;
		char buf[bufsize];
		std::cin.getline(buf, bufsize);
		std::string line(buf);
		int len = line.length();

		std::vector<std::string> stompFrames = convertToStompFrame(line);

		for (std::string &frame : stompFrames)
		{
			std::lock_guard<std::mutex> lock(mtx);
			// Send the frame to the server
			if (!connectionHandler.sendLine(frame))
			{
				std::cerr << "Failed to send frame to server.\n";
				stopThreadsServer = true;
			}

			if (starts_with(line, "logout"))
			{
				stopThreadsServer = true;
			}
		}
	}
}

// Server thread function
void readFromServer(ConnectionHandler &connectionHandler)
{
	while (!stopThreadsServer)
	{
		std::string serverResponse;
		if (!connectionHandler.getLine(serverResponse))
		{
			std::cout << "Disconnected. Exiting...\n"
					  << std::endl;
			break;
		}

		std::lock_guard<std::mutex> lock(mtx);
		/*
		//  Process server response
		std::cout << "Server: " << serverResponse << std::endl;
		if (starts_with(serverResponse, "RECEIPT"))
		{
			std::cout << "Received acknowledgment from server.\n";
		}
		else if (starts_with(serverResponse, "ERROR"))
		{
			std::cerr << "Error received from server: " << serverResponse << std::endl;
		}
		*/
		int len = serverResponse.length();
		// A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
		// we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
		serverResponse.resize(len - 1);
		std::cout << "Reply: " << serverResponse << " " << len << " bytes " << std::endl
				  << std::endl;
		if (serverResponse == "bye")
		{
			std::cout << "Exiting...\n"
					  << std::endl;
			break;
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
	delete idInChannel;
	delete namesAndPasswords;
	return 0;
}
