#include "Server.hpp"

//QUIT
void Server::quitCommand(Client *client, std::vector<std::string> params)
{
    std::string reason;

    if (params.empty()  || params[0].empty())
        reason = "Quit"; //default to quit 
    else 
        reason = params[0];
    
    // if (!reason.empty() && reason[0] == ':')
    //     reason = reason.substr(1);//remove the : 

    //build the msg 
    std::string msg = prefix(*client) + " QUIT :" + reason;
    //notify channels + clean up
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); it++)
    {
        Channel *ch = it->second;
        if(ch->isMember(client->fd))
        {
            broadcast(ch, msg, client->fd);
            ch->removeMember(client->fd);
            ch->removeOperator(client->fd);
        }
    }
    send_to_client(client->fd, "ERROR :Closing Link: " + client->hostname + " (Quit: " + reason + ")");
    remove_client(client->fd);
   //test in mac
}

//TOPIC
void Server::topicCommand(Client *client, std::vector<std::string> params)
{
    if (params.empty())
    {
        reply(client, "461", "TOPIC", "Not enough parameters");
        return;
    }

    std::string chanName = params[0];

    Channel *channel = findChannel(chanName);
    if (!channel)
    {
        reply(client, "403", chanName, "No such channel");
        return;
    }
    if (!channel->isMember(client->fd))
    {
        reply(client, "442", chanName, "You're not on that channel");
        return;
    }

    // read topic mode 
    if (params.size() == 1)
    {
        if (channel->getTopic().empty())
            send_to_client(client->fd, ":" + SERVER_NAME + " 331 " + client->nickname + " " + chanName + " :No topic is set");
        else
            send_to_client(client->fd, ":" + SERVER_NAME + " 332 " + client->nickname + " " + chanName + " :" + channel->getTopic());
        return;
    }

    if (channel->isTopicRestricted() && !channel->isOperator(client->fd))
    {
        reply(client, "482", chanName, "You're not channel operator");
        return;
    }
    
    //set topic mode 
    std::string newTopic = params[1];
    channel->setTopic(newTopic);

    std::string msg = prefix(*client) + " TOPIC " + chanName  + " :" + newTopic;
    broadcast(channel, msg, -1);
    //check if the 333 topicwhotime is necessary
}

//PRVMSG
//locate targets then send a msg to each one if valid ofc
void send_to_one_target(Client *client, const std::string &target, const std::string &text);
{

}

std::vector<std::string> Server::splitTargets(std::string &targets)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t comma_pos;

    while (start <= targets.size())
    {
        comma_pos = targets.find(',', start;)
        std::string target;
        if (comma_pos == std::string::npos)
        {
            target = targets.substr(start);
            start = targets.size() + 1;
        }
        else 
        {
            target = targets.substr(start, comma_pos - start);
            start = targets.size() + 1;
        }
        if (!target.empty())
            result.push_back(target);
    }
    return result;
}

void Server::privmsgCommand(Client *client, std::vector<std::string> params)
{
    if (params.empty())
    {
        reply(client, "411", "", "No recipient given (PRIVMSG)");
        return;
    }
    if (params.size() < 2 || params[1].empty())
    {
        reply(client, "412", "", "No text to send");
        return;
    }

    std::string text = params[1];
    std::vector<std::sting> targetlist = splitTargets(params[0]);

    for(size_t i = 0; i < targetlist.size(); i++)
        send_to_one_target(client, targetlist[i], text);
}