#include "Server.hpp"

void Server::HandleQuit(Client &client, const command &cmd)
{
	std::string reason = "Client Quit";
	if (!cmd.params.empty())
	{
		reason = cmd.params[0];
		for (size_t i = 1; i < cmd.params.size(); ++i)
			reason += " " + cmd.params[i];
	}
	DisconnectClient(client.GetFd(), reason);
}
