#include "Server.hpp"

void Server::HandleCap(Client &client, const command &cmd)
{
	if (!cmd.params.empty() && cmd.params[0] == "LS")
		SendReply(client.GetFd(), ":ft_ircserv CAP * LS :");
}
