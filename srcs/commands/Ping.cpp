#include "Server.hpp"

void Server::HandlePing(Client &client, const command &cmd)
{
	if (cmd.params.empty())
		SendReply(client.GetFd(), ":ft_ircserv 461 " + (client.getNick().empty() ? "*" : client.getNick()) + " PING :Not enough parameters");
	else
		SendReply(client.GetFd(), ":ft_ircserv PONG ft_ircserv :" + cmd.params[0]);
}
