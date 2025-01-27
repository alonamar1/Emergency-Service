#pragma once

#include <string>
#include <map>
#include <vector>
#include "event.h"

class ChannelEvents
{
private:
    std::map<std::string, std::vector<Event>> eventsBySubject;

public:
    ChannelEvents();
    void addEvent(const std::string &subject, const Event &content);
    std::map<std::string, std::vector<Event>> getEvents();
    bool noReportsInChannel(const std::string &channel) const;
};

class DataBaseClient
{
private:
    std::map<std::string, ChannelEvents> userMessages;

public:
    DataBaseClient();
    void addMessage(const std::string &user, const std::string &subject, const std::string &content);
    std::vector<Event> getEvents(const std::string &user, const std::string &subject);
    void addReport(const std::string &user, const std::string &channel, const Event &event);
    void deleteUser(const std::string &user);
    bool noReportsInUser(const std::string &user) const;
    bool checkIfUserHasReports(const std::string &user, const std::string &channel);
    void deleteData();
};
