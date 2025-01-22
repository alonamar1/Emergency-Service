#include <string>
#include <map>
#include <vector>
#include "event.h"

class ChannelEvents {
private:
    std::map<std::string, std::vector<Event>> eventsBySubject;

public:
    void addEvent(const std::string& subject, const Event& content);

};

class DataBaseClient {
private:
    std::map<std::string, ChannelEvents> userMessages;

public:
    DataBaseClient() {}
    void addMessage(const std::string& user, const std::string& subject, const std::string& content);
    std::vector<Event> getEvents(const std::string& user, const std::string& subject);
};