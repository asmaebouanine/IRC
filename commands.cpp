#include "Server.hpp"

//QUIT
void Server::quitCommand(Client *client, std::vector<std::string> params)
{
    std::string reason;

    if (params.empty()  || params[0].empty())
        reason = "Quit"; //default to quit 
    else 
        reason = params[0];

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
}

//PRVMSG
//locate targets then send a msg to each one if valid ofc
void Server::send_to_one_target(Client *client, const std::string &target, const std::string &text)
{
    //channel or client ? valid ? >> send 
    std::string msg = prefix(*client) + " PRIVMSG " + target + " :" + text;

    if (target[0] == '#')
    {
        Channel *channel = findChannel(target);
        if (!channel)
        {
            reply(client, "403", target, "No such channel");
            return;
        }
        if (!channel->isMember(client->fd))
        {
            reply(client, "404", target, "Cannot send to channel");
            return;
        }
        broadcast(channel, msg, client->fd);
        return;
    }
    Client *targetClient = findClientByNick(target);
    if (!targetClient)
    {
        reply(client, "401", target, "No such nick/channel");
        return;
    }
    send_to_client(targetClient->fd, msg);
}


std::vector<std::string> Server::splitTargets(std::string &targets)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t comma_pos;

    while (start <= targets.size())
    {
        comma_pos = targets.find(',', start);
        std::string target;
        if (comma_pos == std::string::npos)
        {
            target = targets.substr(start);
            start = targets.size() + 1;
        }
        else 
        {
            target = targets.substr(start, comma_pos - start);
            start = comma_pos + 1;
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
    std::vector<std::string> targetlist = splitTargets(params[0]);

    for(size_t i = 0; i < targetlist.size(); i++)
        send_to_one_target(client, targetlist[i], text);
}

//MODE
//syntax : MODE <channel> <mode> <params>
void Server::handle_mode(Client *client, Channel *channel, std::vector<std::string> params)
{
    std::string mode = params[1];
    char sign = '+';
    size_t params_index = 2;

    for(size_t i = 0; i < mode.size() ; i++)
    {
        if(mode[i] == '+' || mode[i] == '-')
        {
            sign = mode[i];
            continue;
        }
    if(mode[i] == 'i')
    {
        channel->setInviteOnly(sign == '+');
        std::string msg = prefix(*client) + " MODE " + params[0] + " " + sign + "i" ;
        broadcast(channel, msg, -1);
    }
    else if (mode[i] == 't')
    {
        channel->setTopicRestricted(sign == '+');
        std::string msg = prefix(*client) + " MODE " + params[0] + " " + sign + "t" ;
        broadcast(channel, msg, -1);
    }
    // +/- o 
    else if (mode[i] == 'o')
    {
        if(params_index >= params.size())
        {
            reply(client, "461", "MODE", ":Not enough parameters");
            return;
        }
        Client *target = findClientByNick(params[params_index]);
        if (!target || !channel->isMember(target->fd))
        {
            send_to_client(client->fd, ":" + SERVER_NAME + " 441 " + client->nickname + " " + params[params_index] + " " + channel->getName() + " :They aren't on that channel");
            params_index++;
            continue;
        }
        if (sign == '+') //grant op privilege
        {
            channel->addOperator(target->fd);
            std::string msg = prefix(*client) + " MODE " + params[0] + " +o " + params[params_index];
            broadcast(channel, msg, -1);
        }
        else
        {
            channel->removeOperator(target->fd);
            std::string msg = prefix(*client) + " MODE " + params[0] + " -o " + params[params_index];
            broadcast(channel, msg, -1);
        }
        params_index++;
    }
    // +/- k
    else if (mode[i] == 'k')
    {
        if (sign == '+')
        {    
            if(params_index >= params.size())
            {
                reply(client, "461", "MODE", ":Not enough parameters");
                return;
            }
            channel->setKey(params[params_index]);
            std::string msg = prefix(*client) + " MODE " + params[0] + " +k " + params[params_index];
            broadcast(channel, msg, -1);
            params_index++;
        }
        else
        {
            channel->setKey("");
            std::string msg = prefix(*client) + " MODE " + params[0] + " -k";
            broadcast(channel, msg, -1);
        }
    }
    // +/- l
    else if (mode[i] == 'l')
    {
        if(sign == '+')
        {
            if(params_index >= params.size())
            {
                reply(client, "461", "MODE", "Not enough parameters");
                return;
            }
            int limit = std::atoi(params[params_index].c_str());
            if (limit <= 0)
            {
                reply(client, "696", "MODE", "Invalid limit mode parameter. Syntax: <limit>");
                params_index++;
                continue;
            }
            channel->setUserLimit(limit);
            std::string msg = prefix(*client) + " MODE " + params[0] + " +l " + params[params_index];
            broadcast(channel, msg, -1);
            params_index++;
        }
        else 
        {
            channel->setUserLimit(0);
            std::string msg = prefix(*client) + " MODE " + params[0] + " -l";
            broadcast(channel, msg, -1);
        }
    }
    else 
        reply(client, "472", std::string(1, mode[i]), "is unknown mode char to me");
    }
}

void Server::modeCommand(Client *client, std::vector<std::string> params)
{
    //parse
    if (params.empty())
    {
        reply(client, "461", "MODE", "Not enough parameters");
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
    if (!channel->isOperator(client->fd))
    {
        reply(client, "482", chanName, "You are not channel operator");
        return;
    }
    
    //what mode is it?
    if (params.size() < 2)
    {
        std::string curr_mode = "+";
        if(channel->isInviteOnly())
            curr_mode += "i";
        if(channel->isTopicRestricted()) 
            curr_mode += "t";
        if(!channel->getKey().empty())
            curr_mode += "k";
        send_to_client(client->fd, ":" + SERVER_NAME + " 324 " + client->nickname + " " + chanName + " " + curr_mode);
        return;
    }
    //valid mode ? handle it     
    handle_mode(client, channel ,params);   
}