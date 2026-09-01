#include "Server.hpp"

void Server::HandleInvite(Client &client, const command &cmd)
{
    if (cmd.params.size() < 2)
    {
        SendReply(client.GetFd(), "461 INVITE :Not enough parameters");
        return;
    }

    std::string targetNick = cmd.params[0];
    std::string channelName = cmd.params[1];

    Channel *chan = GetChannelByName(channelName);

    if (!chan)
    {
        SendReply(client.GetFd(), "403 " + channelName + " :No such channel");
        return;
    }

    if (!chan->isMember(&client))
    {
        SendReply(client.GetFd(), "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (chan->isInviteOnly() && !chan->isOperator(&client))
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

    if (chan->isMember(target))
    {
        SendReply(client.GetFd(), "443 " + targetNick + " " + channelName + " :is already on channel");
        return;
    }

    // Give the target permission to join the channel
    chan->invite(target);

    // Tell the inviter that the invitation was successful
    SendReply(client.GetFd(),
        "341 " + client.getNick() + " " + targetNick + " " + channelName);

    // Send the INVITE message to the target
    std::string inviteMsg =
        ":" + client.getNick() + "!" + client.getUser() +
        "@" + client.getHost() +
        " INVITE " + targetNick + " :" + channelName;

    SendReply(target->GetFd(), inviteMsg);
}
