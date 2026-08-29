#include "Server.hpp"

bool Server::_Signal = false;

Server::Server()
{
	this->_Port = 0;
	this->_Password = "";
	this->_SerSocketFd = -1;
}

Server::Server(const Server& other)
{
	*this = other;
}

Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		this->_Port = other._Port;
		this->_Password = other._Password;
		this->_SerSocketFd = other._SerSocketFd;
		this->_clients = other._clients;
		this->_fds = other._fds;
		this->_channels = other._channels;
	}
	return *this;
}

Server::~Server()
{
}

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << YEL << "Signal received! Shutting down server..." << WHI << std::endl;
	Server::_Signal = true;
}

void Server::SetPassword(const std::string& pass)
{
	this->_Password = pass;
}

std::string Server::GetPassword() const
{
	return this->_Password;
}

void Server::SetPort(int port)
{
	this->_Port = port;
}

int Server::GetPort() const
{
	return this->_Port;
}

void Server::CloseFds()
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		std::cout << RED << "Client <" << it->first << "> Disconnected" << WHI << std::endl;
		close(it->first);
	}
	if (this->_SerSocketFd != -1)
	{
		std::cout << RED << "Server <" << this->_SerSocketFd << "> Closed" << WHI << std::endl;
		close(this->_SerSocketFd);
	}
}

void Server::ClearClients(int fd)
{
	for (size_t i = 0; i < this->_fds.size(); ++i)
	{
		if (this->_fds[i].fd == fd)
		{
			this->_fds.erase(this->_fds.begin() + i);
			break;
		}
	}
	Client *cli = GetClientByFd(fd);
	if (cli)
	{
		std::vector<Channel>::iterator it = this->_channels.begin();
		while (it != this->_channels.end())
		{
			it->removeMember(cli);
			if (it->isEmpty())
			{
				it = this->_channels.erase(it);
				continue;
			}
			else
			{
				++it;
			}
		}
		this->_clients.erase(fd);
	}
}

void Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;

	std::memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET;
	add.sin_port = htons(this->_Port);
	add.sin_addr.s_addr = INADDR_ANY;

	this->_SerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_SerSocketFd == -1)
		throw(std::runtime_error("failed to create socket"));

	int en = 1;
	if (setsockopt(this->_SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));

	if (fcntl(this->_SerSocketFd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));

	if (bind(this->_SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("failed to bind socket"));

	if (listen(this->_SerSocketFd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() failed"));

	NewPoll.fd = this->_SerSocketFd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	this->_fds.push_back(NewPoll);
}

void Server::ServerInit()
{
	this->_Port = 4444;
	this->_Password = "";
	SerSocket();

	std::cout << GRE << "Server <" << this->_SerSocketFd << "> Listening on port " << this->_Port << WHI << std::endl;
	std::cout << "Waiting to accept connections..." << std::endl;
}

void Server::ServerInit(int port, const std::string& password)
{
	this->_Port = port;
	this->_Password = password;
	SerSocket();

	std::cout << GRE << "Server <" << this->_SerSocketFd << "> Listening on port " << this->_Port << WHI << std::endl;
	std::cout << "Waiting to accept connections..." << std::endl;
}

void Server::AcceptNewClient()
{
	struct sockaddr_in clientAdd;
	socklen_t len = sizeof(clientAdd);

	std::memset(&clientAdd, 0, sizeof(clientAdd));
	int fd = accept(this->_SerSocketFd, (struct sockaddr *)&clientAdd, &len);
	if (fd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed" << std::endl;
		close(fd);
		return;
	}

	struct pollfd NewPoll;
	NewPoll.fd = fd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	this->_fds.push_back(NewPoll);

	Client cli(fd);
	cli.setIpAdd(inet_ntoa(clientAdd.sin_addr));
	cli.setHost(inet_ntoa(clientAdd.sin_addr));
	this->_clients[fd] = cli;

	std::cout << GRE << "Client <" << fd << "> Connected from IP " << inet_ntoa(clientAdd.sin_addr) << WHI << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];
	std::memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		close(fd);
		ClearClients(fd);
		return;
	}

	Client *cli = GetClientByFd(fd);
	if (!cli)
		return;

	cli->AppendToBuffer(std::string(buff, bytes));

	while (true)
	{
		Client *c = GetClientByFd(fd);
		if (!c) break;
		
		size_t pos = c->GetBuffer().find('\n');
		if (pos == std::string::npos) break;

		std::string line = c->GetBuffer().substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		c->GetBuffer().erase(0, pos + 1);

		if (line.empty())
			continue;

		ParseCommands(*c, line);
	}
}

void Server::ServerRun()
{
	while (Server::_Signal == false)
	{
		int ret = poll(&this->_fds[0], this->_fds.size(), -1);
		if (ret == -1)
		{
			if (Server::_Signal)
				break;
			throw(std::runtime_error("poll() failed"));
		}

		for (size_t i = 0; i < this->_fds.size(); ++i)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				if (this->_fds[i].fd == this->_SerSocketFd)
					AcceptNewClient();
				else
					ReceiveNewData(this->_fds[i].fd);
			}
		}
	}
	CloseFds();
}

Client* Server::GetClientByFd(int fd)
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
		return &it->second;
	return NULL;
}

Client* Server::GetClientByNick(const std::string &nick)
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if (it->second.getNick() == nick)
			return &it->second;
	}
	return NULL;
}

void Server::SendReply(int fd, const std::string &message)
{
	std::string msg = message;
	if (msg.size() < 2 || msg.substr(msg.size() - 2) != "\r\n")
		msg += "\r\n";
	send(fd, msg.c_str(), msg.size(), 0);
}

void Server::ParseCommands(Client &client, const std::string &line)
{
	command cmd = ParseLine(line);
	if (cmd.command.empty())
		return;

	for (size_t i = 0; i < cmd.command.size(); i++)
		cmd.command[i] = std::toupper(cmd.command[i]);

	if (cmd.command == "PASS")
		HandlePass(client, cmd);
	else if (cmd.command == "NICK")
		HandleNick(client, cmd);
	else if (cmd.command == "USER")
		HandleUser(client, cmd);
	else if (cmd.command == "PING")
		HandlePing(client, cmd);
	else if (cmd.command == "CAP")
		HandleCap(client, cmd);
	else if (cmd.command == "QUIT")
		HandleQuit(client, cmd);
	else if (cmd.command == "JOIN")
		HandleJoin(client, cmd);

	CheckRegistration(client);
}

void Server::HandlePass(Client &client, const command &cmd)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();
	if (cmd.params.empty())
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(nick, "PASS"));
	else if (client.isRegistered())
		SendReply(client.GetFd(), ":ft_ircserv 462 " + nick + " :You may not reregister");
	else if (cmd.params[0] != this->_Password)
		SendReply(client.GetFd(), ERR_PASSWDMISMATCH(nick));
	else
		client.setPassOk(true);
}

void Server::HandleNick(Client &client, const command &cmd)
{
	std::string clientNick = client.getNick().empty() ? "*" : client.getNick();
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
		std::string message = ":" + oldNick + "!" + client.getUser() + "@" + (client.getHost().empty() ? client.getIpAdd() : client.getHost()) + " NICK :" + newNick;
		SendReply(client.GetFd(), message);
		BroadcastToSharedChannels(&client, message, &client);
	}
}

void Server::HandleUser(Client &client, const command &cmd)
{
	std::string clientNick = client.getNick().empty() ? "*" : client.getNick();
	if (client.isRegistered())
	{
		SendReply(client.GetFd(), ":ft_ircserv 462 " + clientNick + " :Unauthorized command (already registered)");
		return;
	}

	if (cmd.params.size() < 4)
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "USER"));
		return;
	}

	std::string username = cmd.params[0];
	std::string realname = cmd.params[3];

	if (username.empty() || realname.empty())
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(clientNick, "USER"));
		return;
	}

	client.setUsername(username);
	client.setRealname(realname);
}

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

	if (!chan) // if the channel doesnt exist we create it 
	{
		_channels.push_back(Channel(ChannelName));
        chan = GetChannelByName(ChannelName);
        chan->addMember(&client);
        chan->addOperator(&client); // the creator become operator
	}
	else
	{
		if (chan->isMember(&client))
			return;
		chan->addMember(&client);
	}

	std::string joinMsg = ":" + client.getNick() + " JOIN :" + ChannelName;
	std::set<Client*> members = chan->getMembers();
	for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
		SendReply((*it)->GetFd(), joinMsg);

	// send topic info to the client who join
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
	// send names list to the joiner

}

void Server::CheckRegistration(Client &client)
{
	if (client.isRegistered())
		return;

	if (!client.IsPasswordAccepted())
		return;

	if (client.getNick().empty())
		return;

	if (client.getUser().empty())
		return;

	client.setRegistered(true);

	std::string nick = client.getNick();
	SendReply(client.GetFd(), RPL_WELCOME(nick));
	SendReply(client.GetFd(), ":ft_ircserv 002 " + nick + " :Your host is ft_ircserv, running version 1.0");
	SendReply(client.GetFd(), ":ft_ircserv 003 " + nick + " :This server was created 2026");
	SendReply(client.GetFd(), ":ft_ircserv 004 " + nick + " ft_ircserv 1.0 o itkol");
}

void Server::HandleTopic(Client &client, const command &cmd)
{
    std::string clientNick = client.getNick().empty() ? "*" : client.getNick();
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

    // 1 parameter: View current topic
    if (cmd.params.size() == 1)
    {
        if (chan->getTopic().empty())
            SendReply(client.GetFd(), RPL_NOTOPIC(clientNick, chanName));
        else
            SendReply(client.GetFd(), RPL_TOPIC(clientNick, chanName, chan->getTopic()));
        return;
    }

    // 2 or more parameters: Set / Change topic
    // Check +t mode restrictions: if topic is restricted and client is not an operator
    if (chan->isTopicRestricted() && !chan->isOperator(&client))
    {
        SendReply(client.GetFd(), ERR_CHANOPRIVSNEEDED(clientNick, chanName));
        return;
    }

    std::string newTopic = cmd.params[1];
    for (size_t i = 2; i < cmd.params.size(); ++i)
        newTopic += " " + cmd.params[i];

    chan->setTopic(newTopic);

    // Broadcast topic change to ALL channel members (including sender)
    std::string topicMsg = ":" + client.prefix() + " TOPIC " + chanName + " :" + newTopic;
    chan->broadcast(topicMsg);
}

void Server::HandlePrivmsg(Client &client, const command &cmd)
{
    std::string clientNick = client.getNick().empty() ? "*" : client.getNick();
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
        std::string target = (comma == std::string::npos) ? targets.substr(start) : targets.substr(start, comma - start);
        start = (comma == std::string::npos) ? targets.size() : comma + 1;

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

void Server::HandlePing(Client &client, const command &cmd)
{
	if (cmd.params.empty())
		SendReply(client.GetFd(), ":ft_ircserv 461 " + (client.getNick().empty() ? "*" : client.getNick()) + " PING :Not enough parameters");
	else
		SendReply(client.GetFd(), ":ft_ircserv PONG ft_ircserv :" + cmd.params[0]);
}

void Server::HandleCap(Client &client, const command &cmd)
{
	if (!cmd.params.empty() && cmd.params[0] == "LS")
		SendReply(client.GetFd(), ":ft_ircserv CAP * LS :");
}

void Server::HandleQuit(Client &client, const command &cmd)
{
	std::string reason = cmd.params.empty() ? "Client Quit" : cmd.params[0];
	DisconnectClient(client.GetFd(), reason);
}

void Server::DisconnectClient(int fd, const std::string &reason)
{
	Client *cli = GetClientByFd(fd);
	if (!cli)
		return;

	std::string quitMsg = ":" + cli->prefix() + " QUIT :" + reason;
	BroadcastToSharedChannels(cli, quitMsg, cli);

	std::cout << RED << "Client <" << fd << "> Disconnected (" << reason << ")" << WHI << std::endl;
	close(fd);
	ClearClients(fd);
}

void Server::BroadcastToSharedChannels(Client *client, const std::string &message, Client *exclude)
{
	if (!client)
		return;

	std::set<Client*> recipients;
	for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
	{
		if (it->isMember(client))
		{
			std::set<Client*> members = it->getMembers();
			for (std::set<Client*>::iterator mit = members.begin(); mit != members.end(); ++mit)
			{
				if (*mit != exclude && *mit != client)
					recipients.insert(*mit);
			}
		}
	}

	for (std::set<Client*>::iterator rit = recipients.begin(); rit != recipients.end(); ++rit)
	{
		SendReply((*rit)->getFd(), message);
	}
}

Channel* Server::GetChannelByName(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i].getName() == name)
			return &_channels[i];
	}
	return NULL;
}
