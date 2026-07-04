#include "Server.hpp"

void Server::server_setup()
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cout << "socket failed \n";
        return;
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cout << "setsockopt failed \n";
        close(server_fd);
        server_fd = -1;
        return;
    }
    
    if (fcntl(server_fd , F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl failed \n";
        close(server_fd);
        server_fd = -1;
        return;
    }


    addr.sin_family = AF_INET;
    addr.sin_port = htons(s_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        std::cout << "binding failed \n";
        close(server_fd);
        server_fd = -1;
        return;
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cout << "listening failed \n";
        close(server_fd);
        server_fd = -1;
        return;
    }

    pollfd server;
    server.fd = server_fd;
    server.events = POLLIN;
    server.revents = 0;

    fds.push_back(server);

    std::cout << "server ready\n";
    return;
}


void Server::server_core()
{

    while (g_running)
    {
    
        if (poll(fds.data(), fds.size(), -1) < 0)
        {
            std::cout << "server shuts down\n";
            break;
        }

        for (size_t i = 0; i < fds.size();)
        {
            size_t current_size = fds.size();

            if (fds[i].revents & (POLLHUP | POLLERR))
            {
                remove_client(fds[i].fd);
                continue; 
            }
            if (fds[i].revents & (POLLIN))
            {
                if (fds[i].fd == server_fd)
                    handle_new_client();
                else
                    handle_client(fds[i].fd);
            }
            if (fds.size() == current_size)
                i++;
        }
    }
}

void Server::run()
{
    server_setup();
    if (server_fd == -1)
        return;
    server_core(); 
}