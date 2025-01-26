#pragma once
#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include "../include/dataBaseClient.h"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <map>


// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol();
    bool starts_with(const std::string &source, const std::string &pre);
    bool compareByDateAndName(const Event &a, const Event &b);
    void sortEvents(std::vector<Event> &events);
    std::vector<std::string> jsonToEvent(std::string filepath);
    void generateSummary(const std::string &channelName, const std::string &userName,
                         const std::string &filePath, const std::vector<Event> &events);
    std::vector<std::string> convertToStompFrame(const std::string &userInput);
    const std::atomic<bool>& getStopThreadsServer();
    ConnectionHandler& getConnectionHandler();
    std::mutex &getMutex();
    void disconnectFromCurrentSocket();
    //void readFromServer();
    void readFromKeyboard();
    std::unordered_map<int, std::string> &getReceiptToMessage();
    DataBaseClient &getUserMessages();
    std::string &getUserName();

private:
    // Receipt ID Counter
    int recipt;

    // ID subscribe Counter
    int id;

    // Object that store the events of the user
    DataBaseClient userMessages;

    // save active user name
    std::string userName;

    // Map to store usernames and passwords
    std::map<std::string, std::string> namesAndPasswords;

    // Map to store channel name and the corresponding ID subscribed to the channel
    std::map<std::string, int> idInChannel;

    // Flag to check if the user is logged in
    std::atomic<bool> login;

    // Map to store receipt ID and the corresponding message
    std::unordered_map<int, std::string> receiptToMessage;

    // Connection handler
    ConnectionHandler *connectionHandler;

    // Server thread
    std::thread *serverThread;

    std::atomic<bool> stopThreadsServer;

    // Mutex for thread synchronization
    std::mutex mtx;
};
