#include "Server.hpp"

void Server::HandlePart(Client &client, const command &cmd)
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
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "PART"));
		return;
	}

	std::string channels = cmd.params[0];
	std::string reason;
	if (cmd.params.size() > 1)
		reason = cmd.params[1];
	else
		reason = "";
	for (size_t i = 2; i < cmd.params.size(); ++i)
		reason += " " + cmd.params[i];

	size_t start = 0;
	while (start < channels.size())
	{
		size_t comma = channels.find(',', start);
		std::string chanName;
		if (comma == std::string::npos)
			chanName = channels.substr(start);
		else
			chanName = channels.substr(start, comma - start);
		if (comma == std::string::npos)
			start = channels.size();
		else
			start = comma + 1;

		if (chanName.empty())
			continue;

		Channel *chan = GetChannelByName(chanName);
		if (!chan)
		{
			SendReply(client.GetFd(), ERR_NOSUCHCHANNEL(clientNick, chanName));
			continue;
		}

		if (!chan->isMember(&client))
		{
			SendReply(client.GetFd(), ERR_NOTONCHANNEL(clientNick, chanName));
			continue;
		}

		// Broadcast PART message to all channel members (including leaving client)
		std::string partMsg = ":" + client.prefix() + " PART " + chanName;
		if (!reason.empty())
			partMsg += " :" + reason;
		chan->broadcast(partMsg);

		// Remove client from channel
		chan->removeMember(&client);

		// If channel is now empty, remove it from _channels vector
		if (chan->isEmpty())
		{
			for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
			{
				if (it->getName() == chanName)
				{
					this->_channels.erase(it);
					break;
				}
			}
		}
	}
}

