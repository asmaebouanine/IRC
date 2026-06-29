#include "Server.hpp"


volatile sig_atomic_t   g_running = 1;

Server::Server(int port,const std::string Password):server_fd(-1),s_port(port), SERVER_PASSWORD(Password), SERVER_NAME("IRC")
{
  
}

Server::~Server()
{
    for (size_t i = 0; i < fds.size(); i++)
        close(fds[i].fd);
}

void Server::signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

bool parse_port(const std::string& str, int& port)
{
    char *end;

    if (str.empty())
        return false;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (!std::isdigit(str[i]))
            return false;
    }

    long val = std::strtol(str.c_str(), &end, 10);
    if (*end != '\0')
        return(false);
    if (val < 1024 || val > 65535) // tcp port number is 16 bit with 2 posiiblities 0 and 1 so the max value is 2^16 = 65535
        return false;
    port = static_cast<int>(val);
    return true;
}

int main(int argc, char **argv)
{
   if(argc != 3)
   {
     std::cout << "you should enter port and password \n";
     return(1);
   }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler);
    signal(SIGTERM, Server::signal_handler);

   int port;
   if(!parse_port(argv[1], port))
   {
        std::cout << "Invalid port range\n";
        return(1);
   }

   Server server(port,argv[2]);
   server.run(); 

}
