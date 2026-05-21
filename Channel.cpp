#include "Channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _key(""),
      _userLimit(0),
      _inviteOnly(false),
      _topicRestricted(false)
      {}
//getters
const std::string& Channel::getName() const
{
    return (_name);
}
const std::string& Channel::getTopic() const
{
    return (_topic);
}
const std::string& Channel::getKey() const
{
    return (_key);
}
int Channel::getUserLimit() const
{
    return (_userLimit);
}
bool Channel::isInviteOnly() const
{
    return (_inviteOnly);
}
bool Channel::isTopicRestricted() const
{
    return (_topicRestricted);
}

//setters
void Channel::setTopic(const std::string& topic)    {_topic = topic;}
void Channel::setKey(const std::string& key)        {_key = key;}
void Channel::setUserLimit(int limit)               {_userLimit = limit;}
void Channel::setInviteOnly(bool val)               {_inviteOnly = val;}
void Channel::setTopicRestricted(bool val)          {_topicRestricted = val;}

//members
void Channel::addMember(int fd)
{
    if (!isMember(fd))
        _members.push_back(fd);
}

void Channel::removeMember(int fd)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == fd)
        {
            _members.erase(_members.begin() + i);
            return ;
        }
    }
}

bool Channel::isMember(int fd) const
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == fd)
            return true;
    }
    return false;
}

//operators
void Channel::addOperator(int fd)
{
    if (!isOperator(fd))
        _operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == fd)
        {
            _operators.erase(_operators.begin() + i);
            return ;
        }
    }
}

bool Channel::isOperator(int fd) const
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == fd)
            return true;
    }
    return false;
}

//invite
void Channel::addInvite(int fd)
{
    if (!isInvited(fd))
        _inviteList.push_back(fd);
}
void Channel::removeInvite(int fd)
{
    for (size_t i = 0; i < _inviteList.size(); i++)
    {
        if (_inviteList[i] == fd)
        {
            _inviteList.erase(_inviteList.begin() + i);
            return ;
        }
    }
}

bool Channel::isInvited(int fd) const
{
    for (size_t i = 0; i < _inviteList.size(); i++)
    {
        if (_inviteList[i] == fd)
            return true;
    }
    return false;
}

//utility
bool Channel::isEmpty() const
{
    return (_members.empty());
}
int Channel::getMemberCount() const
{
    return ((int)_members.size());
}
const std::vector<int>& Channel::getMembers() const
{
    return (_members);
}

