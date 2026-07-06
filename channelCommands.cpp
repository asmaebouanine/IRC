#include "Server.hpp"

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
void Server::operatorHandover(Channel *channel, const std::string &chanName)
{
    if (!channel->isEmpty() && channel->getOperator().empty())
    {
        int nextOp = channel->getMembers()[0];
        channel->addOperator(nextOp);
        Client *newOp = findClientByFd(nextOp);
        if (newOp)
        {
            std::string msg = ":" + SERVER_NAME + " MODE " + chanName + " +o " + newOp->nickname;
            broadcast(channel, msg, -1);
         }
    }
}

void Server::joinCommand(Client *client, std::vector<std::string> params)
{
    if (!client->registered)
    {
        reply(client, "451", "JOIN", "You have not registered");
        return ;
    }
    if (params.empty())
    {
        reply(client, "461", "JOIN", "Not enough parameters");
        return ;
    }
    if (params[0] == "0")
    {
        std::map<std::string, Channel*>::iterator it = channels.begin();
        while (it != channels.end())
        {
            Channel *ch = it->second;
            if (ch->isMember(client->fd))
            {
                std::string partMsg = prefix(*client) + " PART " + ch->getName() + " :Left all channels";
                broadcast(ch, partMsg, -1);
                ch->removeMember(client->fd);
                ch->removeOperator(client->fd);
                if (ch->isEmpty())
                {
                    delete ch;
                    channels.erase(it++);
                    continue ;
                }
                else
                    operatorHandover(ch, ch->getName());
                }
            ++it;
        }
        return ;
    }

    std::vector<std::string> chans = splitTargets(params[0]);
    std::vector<std::string> keys;
    if (params.size() > 1)
        keys = splitTargets(params[1]);

    for (size_t i = 0; i < chans.size(); i++)
    {
        std::string chanName = chans[i];
        std::string currentKey;
        if (i < keys.size())
            currentKey = keys[i];
        else 
            currentKey = "";
        if (chanName.empty() || chanName[0] != '#')
        {
            reply(client, "476", chanName, "Bad Channel Mask");
            continue ;
        }
        
        int count = 0;
        std::map<std::string, Channel*>::iterator it;
        for (it = channels.begin(); it != channels.end(); ++it)
            if (it->second->isMember(client->fd))
                count++;
        if (count >= 25)
        {
            reply (client, "405", chanName, "You have joined too many channels");
            continue;
        }

        Channel *channel = findChannel(chanName);
        bool isNew = (channel == NULL);

        if (isNew)
        {
            channel = new Channel(chanName);
            channels[chanName] = channel;
        }
        else
        {
            if (channel->isMember(client->fd))
                continue ;
            if (channel->isInviteOnly() && !channel->isInvited(client->fd))
            {
                reply(client, "473", chanName, "Cannot join channel (+i)");
                continue ;
            }
            if (!channel->getKey().empty() && channel->getKey() != currentKey)
            {
                reply(client, "475", chanName, "Cannot join channel (+k)");
                continue ;
            }
            if (channel->getUserLimit() > 0 && channel->getMemberCount() >= channel->getUserLimit())
            {
                reply(client, "471", chanName, "Cannot join channel (+l)");
                continue ;
            }
        }
  
        channel->addMember(client->fd);
        if (channel->getMemberCount() == 1)
            channel->addOperator(client->fd);

        channel->removeInvite(client->fd);

        std::string joinMsg = prefix(*client) + " JOIN " + chanName;
        broadcast(channel, joinMsg, -1);

        if (!channel->getTopic().empty())
            send_to_client(client->fd, ":" + SERVER_NAME + " 332 " + client->nickname + " " + chanName + " :" + channel->getTopic());
        send_to_client(client->fd, ":" + SERVER_NAME + " 353 " + client->nickname + " = " + chanName + " :" + namesList(channel));
        send_to_client(client->fd, ":" + SERVER_NAME + " 366 " + client->nickname + " " + chanName + " :End of /NAMES list");
    }
}

void Server::partCommand(Client *client, std::vector<std::string> params)
{
    if (!client->registered)
    {
        reply(client, "451", "PART", "You have not registered");
        return ;
    }
    if (params.empty())
    {
        reply(client, "461", "PART", "Not enough parameters");
        return ;
    }

    std::vector<std::string> chans = splitTargets(params[0]);
    std::string reason = "Leaving";

    if (params.size() > 1)
    {
        reason = "";
        for (size_t i = 1; i < params.size(); i++)
        {
            reason += params[i];
            if (i + 1 < params.size())
                reason += " ";
        }
        if (!reason.empty() && reason[0] == ':')
            reason = reason.substr(1);
    }

    for (size_t i = 0; i < chans.size(); i++)
    {
        std::string chanName = chans[i];
        Channel *channel = findChannel(chanName);

        if (!channel)
        {
            reply(client, "403", chanName, "No such channel");
            continue ;
        }
        if (!channel->isMember(client->fd))
        {
            reply(client, "442", chanName, "You're not on that channel");
            continue ;
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
        else
            operatorHandover(channel, chans[i]);
    }
}

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
    if (channel->isInviteOnly() && !channel->isOperator(client->fd))
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

    channel->addInvite(target->fd);

    send_to_client(client->fd, ":" + SERVER_NAME + " 341 "
        + client->nickname + " " + targetNick + " " + chanName);

    send_to_client(target->fd, prefix(*client) + " INVITE "
        + targetNick + " :" + chanName);
}

void Server::kickCommand(Client *client, std::vector<std::string> params)
{
    if (!client->registered)
    {
        reply(client, "451", "KICK", "You have not registered");
        return ;
    }
    if (params.size() < 2)
    {
        reply(client, "461", "KICK", "Not enough parameters");
        return ;
    }

    std::string chanName = params[0];
    std::vector<std::string> targets = splitTargets(params[1]);
    std::string reason = "Kicked";

    if (params.size() > 2)
    {
        reason = "";
        for (size_t i = 2; i < params.size(); i++)
        {
            reason += params[i];
            if (i + 1 < params.size())
                reason += " ";
        }
        if (!reason.empty() && reason[0] == ':')
            reason = reason.substr(1);
    }

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
    for (size_t i = 0; i < targets.size(); i++)
    {
        std::string targetNick = targets[i];
        Client *target = findClientByNick(targetNick);

        if (!target || !channel->isMember(target->fd))
        {
            reply (client, "441", chanName, targetNick + " is not on that channel");
            continue ;
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
            break;
        }
        else
            operatorHandover(channel, params[0]);
    }
}
