#include "Server.hpp"

void Server::quitCommand(Client *client, std::vector<std::string> params)
{
    std::string reason;

    if (params.empty())
        reason = "Quit"; //default to quit 
    else 
        reason = params[0];
    
    if (!reason.empty() && reason[0] == ':')
        reason = reason.substr(1);//remove the : 
    //build the msg 
    std::string msg = prefix(*client) + "QUIT :" + reason;
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
    // shutdown(client->fd, SHUT_RDWR); 
    //msg properly displayed 
    //test with irssi
}