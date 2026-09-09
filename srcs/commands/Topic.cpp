#include "Server.hpp"

void Server::HandleTopic(Client &client, const command &cmd)
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
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "TOPIC"));
		return;
	}

	std::string chanName = cmd.params[0];
	Channel *chan = GetChannelByName(chanName);
	if (!chan)
	{
		SendReply(client.GetFd(), ERR_NOSUCHCHANNEL(clientNick, chanName));
		return;
	}

	if (!chan->isMember(&client))
	{
		SendReply(client.GetFd(), ERR_NOTONCHANNEL(clientNick, chanName));
		return;
	}

	if (cmd.params.size() == 1)
	{
		if (chan->getTopic().empty())
			SendReply(client.GetFd(), RPL_NOTOPIC(clientNick, chanName));
		else
			SendReply(client.GetFd(), RPL_TOPIC(clientNick, chanName, chan->getTopic()));
		return;
	}

	if (chan->isTopicRestricted() && !chan->isOperator(&client))
	{
		SendReply(client.GetFd(), ERR_CHANOPRIVSNEEDED(clientNick, chanName));
		return;
	}

	std::string newTopic = cmd.params[1];
	for (size_t i = 2; i < cmd.params.size(); ++i)
		newTopic += " " + cmd.params[i];

	chan->setTopic(newTopic);

	std::string topicMsg = ":" + client.prefix() + " TOPIC " + chanName + " :" + newTopic;
	chan->broadcast(topicMsg);
}
