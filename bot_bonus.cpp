
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
        std::cout<< "Connection failed!" << std::endl;
        close(bot_fd);
        return 1;
    }
    std::string auth = "PASS " + password + "\r\n"
                       "NICK TestBot\r\n"
                       "USER Tbot 0 * :just a test boot\r\n";
    send(bot_fd, auth.c_str(), auth.size(), 0);

    std::cout << "[TestdBot] Connected and listening..." << std::endl;
    char buffer[BUFFER_SIZE];
    while (true) {
        std::memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(bot_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }

        std::string raw_msg(buffer);
        std::cout << raw_msg;
    }

    close(bot_fd);
    return 0;
}
