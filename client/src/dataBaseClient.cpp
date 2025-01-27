#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "../include/event.h"
#include "../include/dataBaseClient.h"

ChannelEvents::ChannelEvents() : eventsBySubject(std::map<std::string, std::vector<Event>>())
{
}

void ChannelEvents::addEvent(const std::string &subject, const Event &content)
{
    eventsBySubject[subject].push_back(content);
}

std::map<std::string, std::vector<Event>> ChannelEvents::getEvents()
{
    return eventsBySubject;
}

bool ChannelEvents::noReportsInChannel(const std::string &channel) const
{
    return eventsBySubject.find(channel) == eventsBySubject.end();
}

//-------------------------------------------------------------

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
    userMessages[user].addEvent(channel, event);
}

void DataBaseClient::deleteUser(const std::string &user)
{
    userMessages.erase(user);
}

bool DataBaseClient::noReportsInUser(const std::string &user) const
{
    return userMessages.find(user) == userMessages.end();
}
void DataBaseClient::deleteData()
{
    userMessages.clear();
}

// check if the user exits in database and if he exits check if he has any reports from channel in the data base
/**
 * @brief check if the user exits in database
 *         if he exits check if he has any reports from channel in the data base
 *
 * @param user
 * @param channel
 * @return true
 * @return false
 */
bool DataBaseClient::checkIfUserHasReports(const std::string &user, const std::string &channel)
{
    if (userMessages.find(user) != userMessages.end())
    {
        if (userMessages[user].noReportsInChannel(channel))
        {
            return false;
        }
        return true;
    }
    return false;
}
