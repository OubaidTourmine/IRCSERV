#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>

#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Replies.hpp"

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Server {
private:
	int                            _Port;
	std::string                    _Password;
	int                            _SerSocketFd;
	static bool                    _Signal;
	std::map<int, Client>          _clients;
	std::vector<struct pollfd>     _fds;
	std::vector<Channel> _channels;

public:
	// Orthodox Canonical Form
	Server();
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

	// Setters & Getters
	void SetPassword(const std::string& pass);
	std::string GetPassword() const;
	void SetPort(int port);
	int GetPort() const;

	// Server Engine
	void ServerInit();
	void ServerInit(int port, const std::string& password);
	void SerSocket();
	void AcceptNewClient();
	void ReceiveNewData(int fd);
	void ServerRun();

	// Command Dispatch & Handlers
	void ParseCommands(Client &client, const std::string &line);
	void HandlePass(Client &client, const command &cmd);
	void HandleNick(Client &client, const command &cmd);
	void HandleUser(Client &client, const command &cmd);
	void HandlePrivmsg(Client &client, const command &cmd);
	void HandleTopic(Client &client, const command &cmd);
	void HandlePing(Client &client, const command &cmd);
	void HandleCap(Client &client, const command &cmd);
	void HandleQuit(Client &client, const command &cmd);
	void HandlePart(Client &client, const command &cmd);
	void HandleJoin(Client &client, const command &cmd);
	void HandleKick(Client &client, const command &cmd);
	void HandleInvite(Client &client, const command &cmd);
	void HandleMode(Client &client, const command &cmd);
	void CheckRegistration(Client &client);

	// Channels

	Channel* GetChannelByName(const std::string &name);



	// Signals & Cleanu
	static void SignalHandler(int signum);
	void CloseFds();
	void ClearClients(int fd);
	void DisconnectClient(int fd, const std::string &reason);
	void BroadcastToSharedChannels(Client *client, const std::string &message, Client *exclude = NULL);

	// Client Lookups & Replies
	Client* GetClientByFd(int fd);
	Client* GetClientByNick(const std::string &nick);
	void SendReply(int fd, const std::string &message);
};

#endif

