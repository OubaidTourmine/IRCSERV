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
	for (size_t i = 0; i < this->_clients.size(); ++i)
	{
		std::cout << RED << "Client <" << this->_clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(this->_clients[i].GetFd());
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
	for (size_t i = 0; i < this->_clients.size(); ++i)
	{
		if (this->_clients[i].GetFd() == fd)
		{
			this->_clients.erase(this->_clients.begin() + i);
			break;
		}
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
	this->_clients.push_back(cli);

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

	std::string &inbuf = cli->GetBuffer();
	size_t pos;
	while ((pos = inbuf.find('\n')) != std::string::npos)
	{
		std::string line = inbuf.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		inbuf.erase(0, pos + 1);

		if (line.empty())
			continue;

		ParseCommands(*cli, line);
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
	for (size_t i = 0; i < this->_clients.size(); ++i)
	{
		if (this->_clients[i].GetFd() == fd)
			return &this->_clients[i];
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

	for (size_t i = 0; i < cmd.command.size(); ++i)
		cmd.command[i] = std::toupper(cmd.command[i]);

	std::cout << YEL << "Client <" << client.GetFd() << "> Command: [" << cmd.command << "]" << WHI << std::endl;

	if (cmd.command == "PASS")
		HandlePass(client, cmd);
}

void Server::HandlePass(Client &client, const command &cmd)
{
	if (cmd.params.empty())
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(client.getNick().empty() ? "*" : client.getNick(), "PASS"));
		return;
	}
	if (cmd.params[0] == this->_Password)
	{
		client.setPassOk(true);
		SendReply(client.GetFd(), ":ft_ircserv NOTICE * :Password accepted");
	}
	else
	{
		SendReply(client.GetFd(), ERR_PASSWDMISMATCH(client.getNick().empty() ? "*" : client.getNick()));
	}
}
