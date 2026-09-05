#include "Server.hpp"

void Server::HandleNick(Client &client, const command &cmd)
{
	std::string clientNick;
	if (client.getNick().empty())
		clientNick = "*";
	else
		clientNick = client.getNick();
	if (cmd.params.empty())
	{
		SendReply(client.GetFd(), ":ft_ircserv 431 " + clientNick + " :No nickname given");
		return;
	}

	std::string newNick = cmd.params[0];

	if (newNick.empty() || newNick.length() > 9)
	{
		SendReply(client.GetFd(), ":ft_ircserv 432 " + clientNick + " " + newNick + " :Erroneous nickname");
		return;
	}
	if (!std::isalpha(newNick[0]))
	{
		SendReply(client.GetFd(), ":ft_ircserv 432 " + clientNick + " " + newNick + " :Erroneous nickname");
		return;
	}

	for (size_t i = 1; i < newNick.length(); ++i)
	{
		if (!std::isalnum(newNick[i]) &&
			newNick[i] != '[' &&
			newNick[i] != ']' &&
			newNick[i] != '\\' &&
			newNick[i] != '`' &&
			newNick[i] != '^' &&
			newNick[i] != '_' &&
			newNick[i] != '-')
		{
			SendReply(client.GetFd(), ":ft_ircserv 432 " + clientNick + " " + newNick + " :Erroneous nickname");
			return;
		}
	}

	Client *existing = GetClientByNick(newNick);
	if (existing && existing->getFd() != client.getFd())
	{
		SendReply(client.GetFd(), ":ft_ircserv 433 " + clientNick + " " + newNick + " :Nickname is already in use");
		return;
	}

	std::string oldNick = client.getNick();
	if (oldNick == newNick)
		return;

	client.setNick(newNick);

	if (client.isRegistered())
	{
		std::string host_or_ip;
		if (client.getHost().empty())
			host_or_ip = client.getIpAdd();
		else
			host_or_ip = client.getHost();
		std::string message = ":" + oldNick + "!" + client.getUser() + "@" + host_or_ip + " NICK :" + newNick;
		SendReply(client.GetFd(), message);
		BroadcastToSharedChannels(&client, message, &client);
	}
}
