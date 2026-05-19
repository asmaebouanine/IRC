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

