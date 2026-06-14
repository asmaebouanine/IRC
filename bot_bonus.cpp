
#include <vector>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <poll.h>  
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cctype>

//ADD
#include"Channel.hpp"
#include<map>

#define BUFFER_SIZE 1024

// bool parse_port(const std::string& str, int& port)
// {
//     if (str.empty())
//         return false;

//     for (size_t i = 0; i < str.length(); i++)
//     {
//         if (!std::isdigit(str[i]))
//             return false;
//     }

//     long val = std::strtol(str.c_str(), NULL, 10);
    
//     if (val < 1024 || val > 65535)
//         return false;
//     port = static_cast<int>(val);
//     return true;
// }
// void parse_and_reply(int bot_fd, const std::string& raw_msg) 
// {
//     if (raw_msg.substr(0, 4) == "PING") {
//         std::string pong_reply = "PONG" + raw_msg.substr(4);
//         send(bot_fd, pong_reply.c_str(), pong_reply.size(), 0);
//         return;
//     }

//     size_t privmsg_pos = raw_msg.find(" PRIVMSG ");
//     if (privmsg_pos == std::string::npos)
//         return;

//     std::string sender_nick = "";
//     if (raw_msg[0] == ':') {
//         size_t bang_pos = raw_msg.find('!');
//         if (bang_pos != std::string::npos && bang_pos > 1) {
//             sender_nick = raw_msg.substr(1, bang_pos - 1);
//         }
//     }
//     if (sender_nick == "TestBot")
//         return;
//     size_t target_start = privmsg_pos + 9;
//     size_t target_end = raw_msg.find(" :", target_start);
//     if (target_end == std::string::npos)
//         return;
    
//     std::string target = raw_msg.substr(target_start, target_end - target_start);
//     if (target == "TestBot") {
//         target = sender_nick;
//     }

//     std::string message = raw_msg.substr(target_end + 2);
//     if (!message.empty() && message[message.size() - 1] == '\n') message.erase(message.size() - 1);
//     if (!message.empty() && message[message.size() - 1] == '\r') message.erase(message.size() - 1);


//     if (message.size() >= 4 && message.substr(0, 4) == "!rps") {
//         size_t space_pos = message.find(' ');
//         std::string reply_payload = "";

//         if (space_pos == std::string::npos) 
//         {
//             reply_payload = "PRIVMSG " + target + " :Usage: !rps <rock|paper|scissors>\r\n";
//             send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
//             return;
//         }

//         size_t weapon_start = message.find_first_not_of(' ', space_pos);
//         if (weapon_start == std::string::npos) 
//         {
//             reply_payload = "PRIVMSG " + target + " :Usage: !rps <rock|paper|scissors>\r\n";
//             send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
//             return;
//         }
//         else
//         {
//             std::string user_choice = message.substr(weapon_start);
//             std::string choices[] = {"rock", "paper", "scissors"};
//             std::string bot_choice = choices[std::rand() % 3];
            
//             if (user_choice != "rock" && user_choice != "paper" && user_choice != "scissors") 
//                 reply_payload = "PRIVMSG " + target + " :Pick a valid weapon: rock, paper, or scissors.\r\n";
//             else if (user_choice == bot_choice) 
//                 reply_payload = "PRIVMSG " + target + " : Tie game! We both chose " + bot_choice + ".\r\n";
//             else if ((user_choice == "rock" && bot_choice == "scissors") ||
//                        (user_choice == "paper" && bot_choice == "rock") ||
//                        (user_choice == "scissors" && bot_choice == "paper")) 
//                 reply_payload = "PRIVMSG " + target + " : You win! Your [" + user_choice + "] beats my [" + bot_choice + "].\r\n";
//             else 
//                 reply_payload = "PRIVMSG " + target + "  I win! My [" + bot_choice + "] beats your [" + user_choice + "].\r\n";
//         }
//         send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
//     }
// }
// int main(int argc, char **argv)
// {
//     if(argc != 4)
//     {
//         std::cout << "Usage: ./irc_bot <host> <port> <password>" << std::endl;
//         return 1;
//     }
//     std::string host = argv[1];
//     std::string password = argv[3];
//     int port;

//     if (!parse_port(argv[2], port)) {
//         std::cout << "Error: Invalid port" << std::endl;
//         return 1;
//     }
//     int bot_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (bot_fd < 0) 
//         return 1;

//     sockaddr_in server_addr;
//     std::memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

//     if (connect(bot_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         std::cout<< "Connection failed!" << std::endl;
//         close(bot_fd);
//         return 1;
//     }
//     std::string auth = "PASS " + password + "\r\n"
//                        "NICK TestBot\r\n"
//                        "USER Tbot 0 * :just a test boot\r\n";
//     send(bot_fd, auth.c_str(), auth.size(), 0);

//     std::cout << "[TestdBot] Connected and listening..." << std::endl;
//     char buffer[BUFFER_SIZE];
//     while (true) {
//         std::memset(buffer, 0, BUFFER_SIZE);
//         int bytes_read = recv(bot_fd, buffer, BUFFER_SIZE - 1, 0);
//         if (bytes_read <= 0) {
//             std::cout << "Disconnected from server." << std::endl;
//             break;
//         }

//         std::string raw_msg(buffer);
//         parse_and_reply(bot_fd, raw_msg); 
//     }

//     close(bot_fd);
//     return 0;
// }

bool parse_port(const std::string& str, int& port)
{
    if (str.empty())
        return false;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (!std::isdigit(str[i]))
            return false;
    }

    long val = std::strtol(str.c_str(), NULL, 10);
    if (val < 1024 || val > 65535)
        return false;
    port = static_cast<int>(val);
    return true;
}

void parse_and_reply(int bot_fd, const std::string& raw_msg) 
{
    if (raw_msg.substr(0, 4) == "PING") {
        std::string pong_reply = "PONG" + raw_msg.substr(4);
        send(bot_fd, pong_reply.c_str(), pong_reply.size(), 0);
        return;
    }
    
    size_t privmsg_pos = raw_msg.find(" PRIVMSG ");
    if (privmsg_pos == std::string::npos)
        return;
    std::string sender_nick = "";
    if (raw_msg[0] == ':')
    {
        size_t end_of_nick = raw_msg.find('!');
        if (end_of_nick == std::string::npos || end_of_nick > privmsg_pos) 
            end_of_nick = raw_msg.find(' '); 
        if (end_of_nick != std::string::npos && end_of_nick > 1) 
            sender_nick = raw_msg.substr(1, end_of_nick - 1);
    }

    if (sender_nick == "TestBot")
        return;

    size_t target_start = privmsg_pos + 9;
    size_t target_end = raw_msg.find(" :", target_start);
    if (target_end == std::string::npos)
        return;
    
    std::string target = raw_msg.substr(target_start, target_end - target_start);
    if (target == "TestBot") 
        target = sender_nick;


    if (target.empty())
        return;
    
    std::string message = raw_msg.substr(target_end + 2);
    if (!message.empty() && message[message.size() - 1] == '\n') message.erase(message.size() - 1);
    if (!message.empty() && message[message.size() - 1] == '\r') message.erase(message.size() - 1);

    if (message.size() >= 4 && message.substr(0, 4) == "!rps") 
    {
        size_t space_pos = message.find(' ');
        std::string reply_payload = "";

        if (space_pos == std::string::npos) 
        {
            reply_payload = "PRIVMSG " + target + " :Usage: !rps <rock|paper|scissors>\r\n";
            send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
            return;
        }
        size_t weapon_start = message.find_first_not_of(' ', space_pos);
        if (weapon_start == std::string::npos) 
        {
            reply_payload = "PRIVMSG " + target + " :Usage: !rps <rock|paper|scissors>\r\n";
            send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
            return;
        }

        std::string user_choice = message.substr(weapon_start);

        size_t trailing_space = user_choice.find(' ');
        if (trailing_space != std::string::npos) 
            user_choice = user_choice.substr(0, trailing_space);
        

        std::string choices[] = {"rock", "paper", "scissors"};
        std::string bot_choice = choices[std::rand() % 3];
        
        if (user_choice != "rock" && user_choice != "paper" && user_choice != "scissors") 
            reply_payload = "PRIVMSG " + target + " :Pick a valid weapon: rock, paper, or scissors.\r\n";
        else if (user_choice == bot_choice) 
            reply_payload = "PRIVMSG " + target + " :Tie game! We both chose " + bot_choice + ".\r\n";
         else if ((user_choice == "rock" && bot_choice == "scissors") ||
                   (user_choice == "paper" && bot_choice == "rock") ||
                   (user_choice == "scissors" && bot_choice == "paper")) 
            reply_payload = "PRIVMSG " + target + " :You win! Your [" + user_choice + "] beats my [" + bot_choice + "].\r\n";
         else 
            reply_payload = "PRIVMSG " + target + " :I win! My [" + bot_choice + "] beats your [" + user_choice + "].\r\n";
        
        send(bot_fd, reply_payload.c_str(), reply_payload.size(), 0);
    }
}

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        std::cout << "Usage: ./irc_bot <host> <port> <password>" << std::endl;
        return 1;
    }
    std::string host = argv[1];
    std::string password = argv[3];
    int port;

    if (!parse_port(argv[2], port)) {
        std::cout << "Error: Invalid port" << std::endl;
        return 1;
    }
    int bot_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (bot_fd < 0) 
        return 1;

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (connect(bot_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cout << "Connection failed!" << std::endl;
        close(bot_fd);
        return 1;
    }
    
    std::string auth = "PASS " + password + "\r\n"
                       "NICK TestBot\r\n"
                       "USER Tbot 0 * :just a test boot\r\n";
    send(bot_fd, auth.c_str(), auth.size(), 0);

    std::cout << "[TestBot] Connected and listening..." << std::endl;
    
    char buffer[BUFFER_SIZE];
    while (true) {
        std::memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(bot_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }

        std::string raw_msg(buffer);
        parse_and_reply(bot_fd, raw_msg); 
    }

    close(bot_fd);
    return 0;
}