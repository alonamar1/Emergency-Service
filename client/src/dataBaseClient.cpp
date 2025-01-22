#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "../include/event.h"
#include "../include/dataBaseClient.h"

void ChannelEvents::addEvent(const std::string &subject, const Event &content)
{
    eventsBySubject[subject].push_back(content);
}

std::map<std::string, std::vector<Event>> ChannelEvents::getEvents()
{
    return eventsBySubject;
}

DataBaseClient::DataBaseClient() : userMessages(std::map<std::string, ChannelEvents>()) {}

void DataBaseClient::addMessage(const std::string &user, const std::string &subject, const std::string &content)
{
    userMessages[user].addEvent(subject, content);
}

std::vector<Event> DataBaseClient::getEvents(const std::string &user, const std::string &subject)
{
    return userMessages[user].getEvents().at(subject);
}

void DataBaseClient::addReport(const std::string &user, const std::string &channel, const Event &event)
{
    userMessages[user].getEvents()[channel].push_back(event);
}

void DataBaseClient::deleteUser(const std::string &user)
{
    userMessages.erase(user);
}
