#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "../include/event.h" 

class ChannelEvents {
public:
    std::map<std::string, std::vector<Event>> eventsBySubject;

    void addEvent(const std::string& subject, const Event& content) {
        eventsBySubject[subject].emplace_back(subject, content);
    }

    /*void printMessages() const {
        for (const auto& subjectMessages : messagesBySubject) {
            std::cout << "Subject: " << subjectMessages.first << std::endl;
            for (const auto& message : subjectMessages.second) {
                std::cout << "  Content: " << message.content << std::endl;
            }
        }
    }
    */
};

class DataBaseClient {
public:
    std::map<std::string, ChannelEvents> userMessages;

    DataBaseClient()
    {
        userMessages = std::map<std::string, ChannelEvents>();
    }

    void addMessage(const std::string& user, const std::string& subject, const std::string& content) {
        userMessages[user].addEvent(subject, content);
    }

    void addUser(const std::string& user) {
        userMessages[user] = ChannelEvents();
    }

    std::vector<Event> getEvents(const std::string& user, const std::string& subject) {
        return userMessages[user].eventsBySubject[subject];
    }

/*
    void printAllMessages() const {
        for (const auto& userMessage : userMessages) {
            std::cout << "User: " << userMessage.first << std::endl;
            userMessage.second.printMessages();
        }
    }
    */
};
/*
int main() {
    DataBaseClient dbClient;

    dbClient.addMessage("user1", "subject1", "Hello from user1 on subject1");
    dbClient.addMessage("user1", "subject2", "Hello from user1 on subject2");
    dbClient.addMessage("user2", "subject1", "Hello from user2 on subject1");

    dbClient.printAllMessages();

    return 0;
}*/


