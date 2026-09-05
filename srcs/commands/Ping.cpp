#include "Server.hpp"

void Server::HandlePing(Client &client, const command &cmd)
{
	if (cmd.params.empty())
	{
		std::string nick;
		if (client.getNick().empty())
			nick = "*";
		else
			nick = client.getNick();
		SendReply(client.GetFd(), ":ft_ircserv 461 " + nick + " PING :Not enough parameters");
	}
	else
		SendReply(client.GetFd(), ":ft_ircserv PONG ft_ircserv :" + cmd.params[0]);
}
