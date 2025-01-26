#include "../include/Task.h"
#include "../include/StompProtocol.h"
#include <iostream>
#include <mutex>

extern StompProtocol *stomp;

Task::Task()
{
}

void Task::Run()
{
    while (!stomp->getStopThreadsServer())
    {
        std::string serverResponse;
        if (!(stomp->getConnectionHandler().getLine(serverResponse)))
        {
            std::cout << "Disconnected. Exiting...\n"
                      << std::endl;
            break;
        }

        std::lock_guard<std::mutex> lock(stomp->getMutex());
        int len = serverResponse.length();
        serverResponse.resize(len - 1);

        // Print the server response
        std::cout << "Reply:" << serverResponse << " " << len << " bytes " << std::endl
                  << std::endl;

        if (stomp->starts_with(serverResponse, "RECEIPT"))
        {
            int receiptId = std::stoi(serverResponse.substr(serverResponse.find("receipt-id:") + 11));
            if (stomp->getReceiptToMessage().find(receiptId) != stomp->getReceiptToMessage().end())
            {
                if (stomp->getReceiptToMessage()[receiptId] == "DISCONNECT")
                {
                    std::cout << "Logout successful. Disconnecting...\n";
                    stomp->disconnectFromCurrentSocket();
                }
            }
        }

        if (stomp->starts_with(serverResponse, "ERROR"))
        {
            std::cout << "ERROR FROM THE SERVER: \n"
                      << serverResponse << std::endl
                      << std::endl;
            std::cout << "Disconnecting...\n";
            stomp->disconnectFromCurrentSocket();
        }

        if (stomp->starts_with(serverResponse, "MESSAGE"))
        {
            Event event = Event(serverResponse);

            // update the description of the event
            std::string description;
            std::string line;
            std::istringstream ss(serverResponse);
            while (std::getline(ss, line))
            {
                if (line.find("description") != std::string::npos)
                {
                    description = line.substr(12);
                    event.setDescription(description);
                }
            }
            // Update General Information
            std::map<std::string, std::string> general_information;
            std::string active;
            std::string forces_arrival_at_scene;
            std::istringstream ss2(serverResponse);
            while (std::getline(ss2, line))
            {
                if (line.find("active") != std::string::npos)
                {
                    active = line.substr(8);
                    general_information.insert(std::make_pair("active", active));
                }
                if (line.find("forces arrival at scene") != std::string::npos)
                {
                    forces_arrival_at_scene = line.substr(25);
                    general_information.insert(std::make_pair("forces_arrival_at_scene", forces_arrival_at_scene));
                }
            }
            event.setGeneralInformation(general_information);
            // Add the event to the user's messages
            stomp->getUserMessages().addReport(stomp->getUserName(), event.get_channel_name(), event);
        }
    }
}

Task::~Task()
{
}
