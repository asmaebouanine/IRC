#include "Server.hpp"

std::string Server::prefix(Client &c)
{
    return ":" + c.nickname + "!" + c.username + "@" + c.hostname;
}

void Server::reply(Client *client, const std::string &code, const std::string &command, const std::string &message)
{
    std::string msg = ":" + SERVER_NAME + " " + code;

    std::string target;

    if (!client->nickname.empty())
        target = client->nickname;
    else
        target = "*";

    msg += " " + target;

    if (!command.empty())
        msg += " " + command;

    msg += " :" + message;

    if (!send_to_client(client->fd, msg))
        remove_client(client->fd);
}

bool Server::send_to_client(int fd, std::string msg)
{
    msg += "\r\n";
    if(send(fd, msg.c_str(), msg.size(), 0) == -1)
    {
        return false;
    }
    return true;
}

/* ---------------- CLIENT HANDLING ---------------- */
void Server::handle_new_client()
{
   
    while (true)
    {
         sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
        if (client_fd < 0)
            break;

        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
        {
            close(client_fd);
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)))
        {
            close(client_fd);
            continue;
        }

        pollfd p;
        p.fd = client_fd;
        p.events = POLLIN;
        p.revents = 0;
        fds.push_back(p);

        Client c;
        c.fd = client_fd;
        c.pass_ok = false;
        c.nick_set = false;
        c.user_set = false;
        c.registered = false;
        c.hostname = ip;

        clients.push_back(c);
    }
}

void Server::handle_client(int client_fd)
{
    char buff[1024];
    ssize_t bytes = recv(client_fd, buff, sizeof(buff) - 1, 0);

    if (bytes <= 0)
    {
        remove_client(client_fd);
        return;
    }

    buff[bytes] = '\0';

    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].fd == client_fd)
        {
            clients[i].buffer.append(buff, bytes);
            if (clients[i].buffer.size() > 1024 && clients[i].buffer.find("\n") == std::string::npos)
            {
                std::cout << "Client buffer overflowed, dropping connection.\n";
                remove_client(client_fd);
                return; // Exit immediately since the client object and fd are now destroyed
            }
            check_buffer(&clients[i]);
            break;
        }
    }
}

void Server::remove_client(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].fd == fd)
        {
            clients.erase(clients.begin() + i);
            break;
        }
    }
     for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break;
        }
    }
    close(fd);
}