#include "Server.hpp"

void Server::HandlePass(Client &client, const command &cmd)
{
	std::string nick;
	if (client.getNick().empty())
		nick = "*";
	else
		nick = client.getNick();
	if (cmd.params.empty())
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(nick, "PASS"));
	else if (client.isRegistered())
		SendReply(client.GetFd(), ":ft_ircserv 462 " + nick + " :You may not reregister");
	else if (cmd.params[0] != this->_Password)
		SendReply(client.GetFd(), ERR_PASSWDMISMATCH(nick));
	else
		client.setPassOk(true);
}
