#include "Server.hpp"

void Server::HandlePart(Client &client, const command &cmd)
{
	std::string clientNick = client.getNick().empty() ? "*" : client.getNick();
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
	std::string reason = (cmd.params.size() > 1) ? cmd.params[1] : "";
	for (size_t i = 2; i < cmd.params.size(); ++i)
		reason += " " + cmd.params[i];

	size_t start = 0;
	while (start < channels.size())
	{
		size_t comma = channels.find(',', start);
		std::string chanName = (comma == std::string::npos) ? channels.substr(start) : channels.substr(start, comma - start);
		start = (comma == std::string::npos) ? channels.size() : comma + 1;

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

