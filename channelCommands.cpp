#include "Server.hpp"

//HELPERS
Channel* Server::findChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it;
    it = channels.find(name);
    if (it != channels.end())
        return (it->second);
    return NULL;
}
Client* Server::findClientByFd(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].fd == fd)
            return (&clients[i]);
    }
        return (NULL);
}
Client* Server::findClientByNick(const std::string& nick)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].nickname == nick)
            return (&clients[i]);
    }
    return (NULL);
}
void Server::broadcast(Channel* ch, const std::string& msg, int excludeFd)
{
    const std::vector<int> &members = ch->getMembers();
    for (size_t i = 0; i < members.size(); i++)
        if (members[i] != excludeFd)
            send_to_client(members[i], msg);
}
std::string Server::namesList(Channel *ch)
{
    std::string list;
    const std::vector<int> &members = ch->getMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        Client *c = findClientByFd(members[i]);
        if (!c)
            continue;
        if (ch->isOperator(c ->fd))
            list += "@";
        list += c->nickname;
        if (i + 1 < members.size())
            list += " ";
    }
    return list;
}
//JOIN
void Server::joinCommand(Client *client, std::vector<std::string> params)
{
    if (!client->registered)
    {
        reply(client, "451 ", "JOIN", "You have not registered");
        return ;
    }
    if (params.empty())
    {
        reply(client, "461 ", "JOIN", "Not enough parameters");
        return ;
    }
    //parse "JOIN #channel" or "JOIN #channel key"
    std::string chanName = params[0];     //#room
    std::string key;      //"" or password

    if (params.size() > 1)
        key = params[1];
    else
        key = "";

    //channel name must start with #
    if (chanName.empty() || chanName[0] != '#')
    {
        reply(client, "476 ", chanName, "Bad Channel Mask");
        return ;
    }
    //Look in server if the channel exist (if exist false)
    Channel *channel = findChannel(chanName);
    bool isNew = (channel == NULL);
    
    if (isNew)
        channel = new Channel(chanName);
    else
    {
        //check if the client exist
        if (channel->isMember(client->fd))
            return ;

        //invite only check
        if (channel->isInviteOnly() && !channel->isInvited(client->fd))
        {
            reply(client, "473 ", chanName, "Cannot join channel (+i)");
                return ;
        }

        //key check
        if (!channel->getKey().empty() && channel->getKey() != key)
        {
            reply(client, "475 ", chanName, "Cannot join channel (+k)");
                return ;
        }
        
        //user limit check
        if (channel->getUserLimit() > 0 && channel->getMemberCount() >= channel->getUserLimit())
        {
            reply(client, "471 ", chanName, "Cannot join channel (+l)");
                return ;
        }
    }
    //save new channel in server map
    if (isNew)
        channels[chanName] = channel;
    //add client to channel
    channel->addMember(client->fd);
    //first member -> becomes operator
    if (channel->getMemberCount() == 1)
        channel->addOperator(client->fd);

    //consume invite
    channel->removeInvite(client->fd);
    //broadcast JOIN to everyone in channel
    std::string joinMsg = prefix(*client) + " JOIN " + chanName;
    broadcast(channel, joinMsg, -1);
    //send topic
    if (!channel->getTopic().empty())
        send_to_client(client->fd, ":" + SERVER_NAME + " 332 " + client->nickname + " " + chanName + " :" + channel->getTopic());

    else
        send_to_client(client->fd, ":" + SERVER_NAME + " 331 " + client->nickname + " " + chanName + " :No topic is set");
    //send name list
    send_to_client(client->fd, ":" + SERVER_NAME + " 353 " + client->nickname + " = " + chanName + " :" + namesList(channel));
    //end of names
    send_to_client(client->fd, ":" + SERVER_NAME + " 366 " + client->nickname + " " + chanName + " :End of /NAMES list");
}
//PART
void Server::partCommand(Client *client, std::vector<std::string> params)
{
    if (params.empty())
    {
        reply(client, "461", "PART", "Not enough parameters");
        return ;
    }

    std::string chanName = params[0];
    std::string reason;
    if (params.size() > 1)
        reason = params[1];
    else
        reason = "Leaving";

    if (!reason.empty() && reason[0] == ':')
        reason = reason.substr(1);
    
    Channel *channel = findChannel(chanName);
    if (!channel)
    {
        reply(client, "403", chanName, "No such channel");
        return ;
    }
    if (!channel->isMember(client->fd))
    {
        reply(client, "442", chanName, "You're not on that channel");
        return ;
    }

    std::string partMsg = prefix(*client) + " PART " + chanName + " :" + reason;
    broadcast(channel, partMsg, -1);

    channel->removeMember(client->fd);
    channel->removeOperator(client->fd);

    if (channel->isEmpty())
    {
        channels.erase(chanName);
        delete channel;
    }
}
//INVITE
void Server::inviteCommand(Client *client, std::vector<std::string> params)
{
    if (params.size() < 2)
    {
        reply(client, "461", "INVITE", "Not enough parameters");
        return ;
    }

    std::string targetNick = params[0];
    std::string chanName = params[1];

    Channel *channel = findChannel(chanName);
    if (!channel)
    {
        reply(client, "403", chanName, "No such channel");
        return ;
    }
    if (!channel->isMember(client->fd))
    {
        reply(client, "442", chanName, "You're not on that channel");
        return ;
    }
    if (!channel->isOperator(client->fd))
    {
        reply(client, "482", chanName, "You're not channel operator");
        return ;
    }

    Client *target = findClientByNick(targetNick);
    if (!target)
    {
        reply(client, "401", targetNick, "No such nick");
        return ;
    }
    if (channel->isMember(target->fd))
    {
        reply(client, "443", chanName, targetNick + " is already on channel");
        return ;
    }

    channel ->addInvite(target->fd);

    send_to_client(client->fd, ":" + SERVER_NAME + " 341"
        + client->nickname + " " + targetNick + " " + chanName);

    send_to_client(target->fd, prefix(*client) + " INVITE "
        + targetNick + " :" + chanName);
}
//KICK
void Server::kickCommand(Client *client, std::vector<std::string> params)
{
    if (params.size() < 2)
    {
        reply(client, "461", "KICK", "Not enough parameters");
        return ;
    }

    std::string chanName = params[0];
    std::string targetNick = params[1];
    std::string reason;

    if (params.size() > 2)
        reason = params[2];
    else
        reason = "Kicked";

    if (!reason.empty() && reason[0] == ':')
        reason = reason.substr(1);

    Channel *channel = findChannel(chanName);
    if (!channel)
    {
        reply(client, "403", chanName, "No such channel");
        return ;
    }
    if (!channel->isMember(client->fd))
    {
        reply(client, "442", chanName, "You're not that channel");
        return ;
    }
    if (!channel->isOperator(client->fd))
    {
        reply(client, "482", chanName, "You're not channel operator");
        return ;
    }

    Client *target = findClientByNick(targetNick);
    if (!target || !channel->isMember(target->fd))
    {
        reply (client, "441", chanName, targetNick + " is not on that channel");
        return ;
    }

    std::string kickMsg = prefix(*client) + " KICK " + chanName
                        + " " + targetNick + " :" + reason;
    broadcast(channel, kickMsg, -1);

    channel->removeMember(target->fd);
    channel->removeOperator(target->fd);

    if (channel->isEmpty())
    {
        channels.erase(chanName);
        delete channel;
    }
}