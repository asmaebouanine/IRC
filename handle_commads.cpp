#include "Server.hpp"


void Server::try_register(Client *client)
{
    if (client->pass_ok && client->nick_set && client->user_set && !client->registered)
    {
        client->registered = true;

        std::string identity = client->nickname + "!" + client->username + "@" + client->hostname;
        std::string rpl_welcome = "Welcome to the Internet Relay Network, " + identity;
        reply(client, "001", "", rpl_welcome);

        std::string rpl_yourhost = "Your host is "+ SERVER_NAME + ", running version 1.0";
        reply(client, "002", "", rpl_yourhost);

        std::time_t now = std::time(NULL);
        std::string time_str = std::ctime(&now);
        
        if (!time_str.empty() && time_str[time_str.size() - 1] == '\n')
            time_str.erase(time_str.size() - 1);

        std::string rpl_created = "This server was created " + time_str;
        reply(client, "003", "", rpl_created);
      
        std::string rpl_myinfo = SERVER_NAME + " 1.0 0 itklo";

        reply_params(client, "004", rpl_myinfo);

        std::string pref = prefix(*client);
        std::cout << pref << " has connected to " << SERVER_NAME << std::endl;
    }
}



bool Server::nickname_exists(const std::string &nick)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (irc_equal(clients[i].nickname,nick))
            return true;
    }
    return false;
}
bool Server::is_special_char(char c)
{
    return (c == '[' ||c == ']' ||c == '\\' ||c == '`' ||
            c == '_' ||c == '^' || c == '{' ||c == '|' ||
            c == '}');
}
bool Server::irc_equal(const std::string &a, const std::string &b)
{
    if (a.size() != b.size()) 
        return false;

    for (size_t i = 0; i < a.size(); i++)
    {
        char ca = std::tolower(static_cast<unsigned char>(a[i]));
        char cb = std::tolower(static_cast<unsigned char>(b[i]));

        if (ca == '{')      ca = '[';
        else if (ca == '}') ca = ']';
        else if (ca == '|') ca = '\\';
        else if (ca == '^') ca = '~';

        if (cb == '{')      cb = '[';
        else if (cb == '}') cb = ']';
        else if (cb == '|') cb = '\\';
        else if (cb == '^') cb = '~';

        if (ca != cb) 
            return false;
    }
    return true;
}
bool Server::is_valid_nick(const std::string &nick)
{
    if (nick.empty())
        return false;

    if (nick.size() > 9)
        return false;

    if (!std::isalpha(static_cast<unsigned char>(nick[0])) &&
        !is_special_char(nick[0]))
        return false;

    for (size_t i = 1; i < nick.size(); i++)
    {
        if (!std::isalpha(static_cast<unsigned char>(nick[i])) &&
            !std::isdigit(static_cast<unsigned char>(nick[i])) &&
            !is_special_char(nick[i]) &&
             nick[i] != '-')
        return false;
    }

    return true;
}

void Server::nick_command(Client *client, Command command)
{

    if (command.params.size() <1)
    {
        reply(client, "431", "", "No nickname given");
        return;
    }

    if (!is_valid_nick(command.params[0]))
    {
        reply(client, "432", command.params[0], "Erroneus nickname");
        return;
    }

    if(nickname_exists(command.params[0]) && !irc_equal(client->nickname,command.params[0]) )
    {
        reply(client, "433", command.params[0], "Nickname is already in use");
        return;
    }
    if (client->registered && !irc_equal(client->nickname, command.params[0]))
    {
        std::string old_nick = client->nickname;
        std::map<std::string, Channel*>::iterator it;
        std::vector<int> notified_fds;

        std::string msg = prefix(*client) + " NICK :" + command.params[0];
        
        send_to_client(client->fd, msg); 
        notified_fds.push_back(client->fd);

        for (it = channels.begin(); it != channels.end(); ++it)
        {
            if (!it->second->isMember(client->fd))
                continue;

            std::vector<int> members = it->second->getMembers();
            for (size_t i = 0; i < members.size(); i++)
            {
                bool is_notified = false; 

                for (size_t j = 0; j < notified_fds.size(); j++)
                {
                    if (notified_fds[j] == members[i])
                    {
                        is_notified = true;
                        break;
                    }
                }

                if (!is_notified)
                {
                    send_to_client(members[i], msg);
                    notified_fds.push_back(members[i]);
                }
            }
        }
    }
    client->nickname = command.params[0];
    client->nick_set = true;
    try_register(client);
}

void Server::pass_command(Client *client, Command command)
{
    
    if(client->registered)
    {
        reply(client, "462", "", "You may not reregister");
        return;
    }
  
    if (command.params.size() < 1)
    {
        reply(client, "461", "PASS", "Not enough parameters");
        return;
    }

    if (command.params[0] != SERVER_PASSWORD)
    {
        reply(client, "464", "", "Password incorrect");
        client->pass_ok = false;
        return;
    }
    client->pass_ok = true;
}


void Server::user_command(Client *client, Command command)
{
    if (command.params.size() < 4)
    {
        reply(client, "461", "USER", "Not enough parameters");
        return;
    }
    if (client->registered)
    {
        reply(client, "462", "", "You may not reregister");
        return;
    }

    client->username = command.params[0];
    client->realname = command.params.back();

    
    client->user_set = true;
    try_register(client);
}


void Server::capitalize_command(std::string &command)
{
    for(size_t i = 0 ; i < command.size(); i++)
    {
       command[i] =  std::toupper(static_cast<unsigned char>(command[i]));
    }
}

void Server::handle_command(Client *client, Command command)
{
    if (command.cmd.empty())
        return;

    if (command.cmd == "PASS")
    {
        pass_command(client, command);
        return;
    }

    if (!client->pass_ok)
    {
        reply(client, "451", "", "You have not registered");
        return;
    }
    if (!client->registered && command.cmd != "NICK" && command.cmd != "USER" && command.cmd != "QUIT")
    {
        reply(client, "451", command.cmd, "You have not registered");
        return;
    }

    if (command.cmd == "NICK")
        nick_command(client, command);
    else if (command.cmd == "USER")
        user_command(client, command);
    else if (command.cmd == "JOIN")
        joinCommand(client, command.params);
    else if (command.cmd == "PART")
        partCommand(client, command.params);
    else if (command.cmd == "INVITE")
        inviteCommand(client, command.params);
    else if (command.cmd == "KICK")
        kickCommand(client, command.params);

    else if (command.cmd == "QUIT")
        quitCommand(client, command.params);
    else if (command.cmd == "TOPIC")
        topicCommand(client, command.params);
    else if (command.cmd == "PRIVMSG")
        privmsgCommand(client, command.params);
    else if (command.cmd == "MODE")
        modeCommand(client, command.params);
    else
        reply(client, "421", command.cmd, "Unknown command");
}

Command Server::dispatch_user(tmp_cmd tmp)
{
    Command command;
    size_t pos;
    std::string param;

    std::string before_colon;
    std::string after_colon;
    bool has_colon = false;



    command.cmd = tmp.cmd;
    size_t colon_index = tmp.arg.find(":");

    if(colon_index != std::string::npos)
    {
        before_colon = tmp.arg.substr(0, colon_index);
        after_colon = tmp.arg.substr(colon_index + 1); 
        has_colon = true;
    }
    else
       before_colon = tmp.arg;

    while ((pos = before_colon.find_first_not_of(" \t")) != std::string::npos)
    {
        before_colon = before_colon.substr(pos);

        size_t space = before_colon.find(' ');

        if (space == std::string::npos)
        {
            command.params.push_back(before_colon);
            break;
        }
        command.params.push_back(before_colon.substr(0, space));
        before_colon = before_colon.substr(space + 1);
    }
    if(has_colon && (!after_colon.empty() || command.cmd == "TOPIC"))
        command.params.push_back(after_colon);

    return(command);
}


Command Server::dispatch_pass_nick(tmp_cmd tmp)
{
    Command command;
    std::string rest;
    size_t pos;

    command.cmd = tmp.cmd;
    rest = tmp.arg;

    while ((pos = rest.find_first_not_of(" \t")) != std::string::npos)
    {
        rest = rest.substr(pos);

        size_t space = rest.find(' ');

        if (space == std::string::npos)
        {
            command.params.push_back(rest);
            break;
        }

        command.params.push_back(rest.substr(0, space));
        rest = rest.substr(space + 1);
    }

    return (command);
}

tmp_cmd Server::command_name(std::string command_)
{
    tmp_cmd tmp;

    std::string rest;
    
    size_t index = command_.find_first_not_of(" \t");

    if(index == std::string::npos)
        return(tmp);

    rest = command_.substr(index);

    size_t space_pos = rest.find(' ');
    if(space_pos == std::string::npos)
    {
        tmp.cmd = rest;
        return(tmp);
    }
    tmp.cmd = rest.substr(0, space_pos);
    tmp.arg = rest.substr(space_pos + 1) ;
    return(tmp);
}

Command Server::parse_command(std::string command_)
{
    tmp_cmd tmp;
    Command command;

    tmp = command_name(command_);
    capitalize_command(tmp.cmd);

    if(tmp.cmd == "PASS" || tmp.cmd == "NICK" || tmp.cmd == "JOIN" || tmp.cmd == "PART" || tmp.cmd == "INVITE" || tmp.cmd == "KICK" || tmp.cmd == "MODE")
        command = dispatch_pass_nick(tmp);
    else if(tmp.cmd == "USER" || tmp.cmd == "QUIT"  || tmp.cmd == "TOPIC" || tmp.cmd == "PRIVMSG")
        command = dispatch_user(tmp);
    else 
        command.cmd = tmp.cmd;
    return(command);
}

void Server::check_buffer(Client *client)
{
    size_t pos;
    std::string command_;
    Command command;

    while ((pos = client->buffer.find("\n")) != std::string::npos)
    {
        command_ = client->buffer.substr(0, pos);
        client->buffer.erase(0, pos + 1);

        if (!command_.empty() && command_[command_.size() - 1] == '\r')
            command_.erase(command_.size() - 1);

        if (command_.size() > 510)
            continue;

        command = parse_command(command_);
        
        int fd = client->fd;
        handle_command(client, command);
        if (get_client(fd) == NULL)
            return;
    }
}
