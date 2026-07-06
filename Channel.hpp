#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Channel
{
    private:
        std::string _name;
        std::string _topic;
        std::string _key;
        int         _userLimit;
        bool        _inviteOnly;
        bool        _topicRestricted;

        std::vector<int> _members;
        std::vector<int> _operators;
        std::vector<int> _inviteList;

    public:
        Channel(const std::string &name);

        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getKey() const;
        const std::vector<int>& getOperator() const;
        int getUserLimit() const;
        bool isInviteOnly() const;
        bool isTopicRestricted () const;

        void setTopic(const std::string& topic);
        void setKey(const std::string& key);
        void setUserLimit(int limit);
        void setInviteOnly(bool val);
        void setTopicRestricted(bool val);

        void addMember(int fd);
        void removeMember(int fd);
        bool isMember(int fd) const;

        void addOperator(int fd);
        void removeOperator(int fd);
        bool isOperator(int fd) const;

        void addInvite(int fd);
        void removeInvite(int fd);
        bool isInvited(int fd) const;

        bool isEmpty() const;
        int getMemberCount() const;
        const std::vector<int>& getMembers() const;

};

#endif