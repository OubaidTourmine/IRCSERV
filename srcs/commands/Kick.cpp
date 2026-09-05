#include "Server.hpp"

void Server::HandleKick(Client &client, const command &cmd)
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

	if (cmd.params.size() < 2)
    {
        SendReply(client.GetFd(), "461 KICK :Not enough parameters");
        return;
    }

    std::string channelName = cmd.params[0];
    std::string targetNick = cmd.params[1];
    std::string comment;

    if (cmd.params.size() >= 3)
        comment = cmd.params[2];
    else
        comment = client.getNick();

    Channel *chan = GetChannelByName(channelName);
    if (!chan)
    {
        SendReply(client.GetFd(), "403 " + channelName + " :No such channel");
        return;
    }

    if (!chan->isOperator(&client))
    {
        SendReply(client.GetFd(), "482 " + channelName + " :You're not channel operator");
        return;
    }

    Client *target = GetClientByNick(targetNick);
    if (!target)
    {
        SendReply(client.GetFd(), "401 " + targetNick + " :No such nick");
        return;
    }

        if (!chan->isMember(target))
    {
        SendReply(client.GetFd(), "441 " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    std::string kickMsg = ":" + client.getNick() + " KICK " + channelName + " " + targetNick + " :" + comment;
    std::set<Client*> members = chan->getMembers();
    for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
        SendReply((*it)->GetFd(), kickMsg);

    chan->removeMember(target);

    if (chan->isEmpty())
    {
        for (size_t i = 0; i < _channels.size(); i++)
        {
            if (_channels[i].getName() == channelName)
            {
                _channels.erase(_channels.begin() + i);
                break;
            }
        }
    }

}