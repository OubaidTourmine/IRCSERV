#include "Server.hpp"

void Server::HandleJoin(Client &client, const command &cmd)
{
	if (cmd.params.empty())
	{
		SendReply(client.GetFd(), "461 JOIN :Not enough parameters");
		return;
	}

	std::string ChannelName = cmd.params[0];

	if (ChannelName.empty() || ChannelName[0] != '#')
	{
		SendReply(client.GetFd(), "403 " + ChannelName + " :No such channel");
		return;
	}

	Channel *chan = GetChannelByName(ChannelName);

	if (!chan)
	{
		_channels.push_back(Channel(ChannelName));
		chan = GetChannelByName(ChannelName);
		chan->addMember(&client);
		chan->addOperator(&client);
	}
	else
	{
		if (chan->isMember(&client))
			return;

		if (chan->isInviteOnly() && !chan->isInvited(&client))
		{
			SendReply(client.GetFd(), ERR_INVITEONLYCHAN(client.getNick(), ChannelName));
			return;
		}

		if (chan->hasKey())
		{
			std::string keyProvided = cmd.params.size() > 1 ? cmd.params[1] : "";
			if (keyProvided != chan->getKey())
			{
				SendReply(client.GetFd(), ERR_BADCHANNELKEY(client.getNick(), ChannelName));
				return;
			}
		}

		if (chan->getUserLimit() != -1 && (int)chan->getMembers().size() >= chan->getUserLimit())
		{
			SendReply(client.GetFd(), ERR_CHANNELISFULL(client.getNick(), ChannelName));
			return;
		}

		chan->consumeInvite(&client);
		chan->addMember(&client);
	}

	std::string joinMsg = ":" + client.getNick() + " JOIN :" + ChannelName;
	std::set<Client*> members = chan->getMembers();
	for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
		SendReply((*it)->GetFd(), joinMsg);

	if (chan->getTopic().empty())
		SendReply(client.GetFd(), "331 " + client.getNick() + " " + ChannelName + " :No topic is set");
	else
		SendReply(client.GetFd(), "332 " + client.getNick() + " " + ChannelName + " :" + chan->getTopic());

	std::string names = "353 " + client.getNick() + " = " + ChannelName + " :";
	for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
	{
		if (chan->isOperator(*it))
			names += "@";
		names += (*it)->getNick();
		std::set<Client*>::iterator next = it;
		if (++next != members.end())
			names += " ";
	}
	SendReply(client.GetFd(), names);
	SendReply(client.GetFd(), "366 " + client.getNick() + " " + ChannelName + " :End of /NAMES list");
}
