#include "Server.hpp"

void Server::HandleUser(Client &client, const command &cmd)
{
	std::string clientNick;
	if (client.getNick().empty())
		clientNick = "*";
	else
		clientNick = client.getNick();
	if (client.isRegistered())
	{
		SendReply(client.GetFd(), ":ft_ircserv 462 " + clientNick + " :Unauthorized command (already registered)");
		return;
	}

	if (cmd.params.size() < 4)
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "USER"));
		return;
	}

	std::string username = cmd.params[0];
	std::string realname = cmd.params[3];

	if (username.empty() || realname.empty())
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "USER"));
		return;
	}

	client.setUsername(username);
	client.setRealname(realname);
}
