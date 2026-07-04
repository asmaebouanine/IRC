#ifndef SERVER_HPP
#define SERVER_HPP

#include "server_client.hpp"
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
#include <ctime> 

#include"Channel.hpp"
#include<map>

extern volatile sig_atomic_t g_running;
struct Command
{
    std::string cmd;
    std::vector<std::string> params;
};
struct tmp_cmd
{
    std::string cmd;
    std::string arg;
};

class Server
{
    private:
        int server_fd;
        int s_port;

        std::vector<pollfd> fds;

        std::vector<Client> clients;

        const std::string SERVER_PASSWORD;
        const std::string SERVER_NAME;

        std::map<std::string, Channel*> channels;

    public:
        Server(int port,const std::string Password);
        ~Server();
        static void signal_handler(int sig);
        void run();

    private:

        Channel*    findChannel(const std::string& name);
        Client*     findClientByFd(int fd);
        Client*     findClientByNick(const std::string& nick);
        void        broadcast(Channel *ch, const std::string& msg, int excludeFd);
        std::string namesList(Channel *ch);

        void        joinCommand(Client *client, std::vector<std::string> params);
        void        partCommand(Client *client, std::vector<std::string> params);
        void        inviteCommand(Client *client, std::vector<std::string> params);
        void        kickCommand(Client *client, std::vector<std::string> params);
        
        void server_setup();
        void server_core();

        void handle_client(int client_fd);
        void handle_new_client(void);

        bool send_to_client(int fd, std::string msg);

        void try_register(Client *client);
        bool is_special_char(char c);
        bool nickname_exists(const std::string &nick);
        bool irc_equal(const std::string &a, const std::string &b);
        bool is_valid_nick(const std::string &nick);
        void nick_command(Client *client, Command command);
        void pass_command(Client *client, Command command);
        // void user_command(Client *client, std::string command);
        void user_command(Client *client, Command command);
        std::string prefix(Client &c);
        Command dispatch_pass_nick(tmp_cmd tmp);
        Command dispatch_user(tmp_cmd tmp);
        tmp_cmd command_name(std::string command_);
        Command parse_command(std::string command_);
        void capitalize_command(std::string &command);
        void handle_command(Client *client,Command command);
        void check_buffer(Client *client);
        int parse_port(std::string port);
        void remove_client(int fd);
        void reply(Client *client, const std::string &code, const std::string &command, const std::string &message);

        //comms
        void quitCommand(Client *client, std::vector<std::string> params);
        void topicCommand(Client *client, std::vector<std::string> params);
        void privmsgCommand(Client *client, std::vector<std::string> params);
        void send_to_one_target(Client *client, const std::string &target, const std::string &text);
        std::vector<std::string> splitTargets(std::string &targets);
        void modeCommand(Client *client, std::vector<std::string> params);
        void handle_mode(Client *client, Channel *channel, std::vector<std::string> params);
};

#endif