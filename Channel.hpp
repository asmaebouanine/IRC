#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Channel
{
    private:
        std::string _name;         //name of channel example #general
        std::string _topic;        //description of the channel
        std::string _key;          //password of the channel
        int         _userLimit;    //maximum of people allowed in the the channel
        bool        _inviteOnly;
        bool        _topicRestricted;

        std::vector<int> _members;   //list of fds every fd represent one connected client[4,5,6]
        std::vector<int> _operators; //list of fds of people represent ADMINS
        std::vector<int> _inviteList; //list of fds of people who have been inveted (not yet joind)

    public:
        //constructor
        Channel(const std::string &name);

        //getters
        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getKey() const;
        int getUserLimit() const;
        bool isInviteOnly() const;
        bool isTopicRestricted () const;

        //setters
        void setTopic(const std::string& topic);
        void setKey(const std::string& key);
        void setUserLimit(int limit);
        void setInviteOnly(bool val);
        void setTopicRestricted(bool val);

        //members
        void addMember(int fd);
        void removeMember(int fd);
        bool isMember(int fd) const;

        //operators
        void addOperator(int fd);
        void removeOperator(int fd);
        bool isOperator(int fd) const;

        //invite
        void addInvite(int fd);
        void removeInvite(int fd);
        bool isInvited(int fd) const;

        //utility
        bool isEmpty() const;
        int getMemberCount() const;
        const std::vector<int>& getMembers() const;

};

#endif