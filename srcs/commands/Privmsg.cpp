#include "Server.hpp"

void Server::HandlePrivmsg(Client &client, const command &cmd)
{
	std::string clientNick;
	if (client.getNick().empty())
		clientNick = "*";
	else
		clientNick = client.getNick();
	if (!client.isRegistered())
	{
		SendReply(client.GetFd(), ERR_NOTREGISTERED(clientNick));
		return;
	}

	if (cmd.params.empty())
	{
		SendReply(client.GetFd(), ERR_NORECIPIENT(clientNick, "PRIVMSG"));
		return;
	}

	if (cmd.params.size() < 2 || cmd.params[1].empty())
	{
		SendReply(client.GetFd(), ERR_NOTEXTTOSEND(clientNick));
		return;
	}

	std::string targets = cmd.params[0];
	std::string text = cmd.params[1];
	for (size_t i = 2; i < cmd.params.size(); ++i)
		text += " " + cmd.params[i];

	size_t start = 0;
	while (start < targets.size())
	{
		size_t comma = targets.find(',', start);
		std::string target;
		if (comma == std::string::npos)
			target = targets.substr(start);
		else
			target = targets.substr(start, comma - start);
		if (comma == std::string::npos)
			start = targets.size();
		else
			start = comma + 1;

		if (target.empty())
			continue;

		std::string formattedMsg = ":" + client.prefix() + " PRIVMSG " + target + " :" + text;

		// Channel target
		if (target[0] == '#' || target[0] == '&')
		{
			Channel *chan = GetChannelByName(target);
			if (!chan)
			{
				SendReply(client.GetFd(), ERR_NOSUCHCHANNEL(clientNick, target));
				continue;
			}

			if (!chan->isMember(&client))
			{
				SendReply(client.GetFd(), ERR_CANNOTSENDTOCHAN(clientNick, target));
				continue;
			}

			chan->broadcast(formattedMsg, &client);
		}
		// User target
		else
		{
			Client *targetClient = GetClientByNick(target);
			if (!targetClient)
			{
				SendReply(client.GetFd(), ERR_NOSUCHNICK(clientNick, target));
				continue;
			}

			SendReply(targetClient->GetFd(), formattedMsg);
		}
	}
}
