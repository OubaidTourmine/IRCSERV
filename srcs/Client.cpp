#include "Client.hpp"
#include <iostream>

Client::Client()
{
	this->_fd = -1;
	this->_nick = "";
	this->_user = "";
	this->_host = "";
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = false;
	this->_passOk = false;
}

Client::Client(int fd)
{
	this->_fd = fd;
	this->_nick = "";
	this->_user = "";
	this->_host = "";
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = false;
	this->_passOk = false;
}

Client::Client(int fd, const std::string& nick, const std::string& user, const std::string& host)
{
	this->_fd = fd;
	this->_nick = nick;
	this->_user = user;
	this->_host = host;
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = true;
	this->_passOk = true;
}

Client::Client(const Client& other)
{
	*this = other;
}

Client& Client::operator=(const Client& other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_nick = other._nick;
		this->_user = other._user;
		this->_host = other._host;
		this->_ipAdd = other._ipAdd;
		this->_buffer = other._buffer;
		this->_registered = other._registered;
		this->_passOk = other._passOk;
		this->_invitedChannels = other._invitedChannels;
	}
	return *this;
}

Client::~Client()
{
}

int Client::getFd() const
{
	return this->_fd;
}

int Client::GetFd() const
{
	return this->_fd;
}

void Client::setFd(int fd)
{
	this->_fd = fd;
}

void Client::SetFd(int fd)
{
	this->_fd = fd;
}

std::string Client::getNick() const
{
	return this->_nick;
}

void Client::setNick(const std::string& nick)
{
	this->_nick = nick;
}

std::string Client::getUser() const
{
	return this->_user;
}

void Client::setUser(const std::string& user)
{
	this->_user = user;
}

std::string Client::getHost() const
{
	return this->_host;
}

void Client::setHost(const std::string& host)
{
	this->_host = host;
}

std::string Client::getIpAdd() const
{
	return this->_ipAdd;
}

void Client::setIpAdd(const std::string& ipAdd)
{
	this->_ipAdd = ipAdd;
}

std::string Client::prefix() const
{
	return this->_nick + "!" + this->_user + "@" + this->_host;
}

bool Client::isPassOk() const
{
	return this->_passOk;
}

void Client::setPassOk(bool passOk)
{
	this->_passOk = passOk;
}

bool Client::isRegistered() const
{
	return this->_registered;
}

void Client::setRegistered(bool value)
{
	this->_registered = value;
}

std::string& Client::GetBuffer()
{
	return this->_buffer;
}

void Client::AppendToBuffer(const std::string& data)
{
	this->_buffer += data;
}

void Client::ClearBuffer()
{
	this->_buffer.clear();
}

void Client::queueOutput(const std::string& msg)
{
	std::cout << "[to " << this->_nick << "] " << msg;
}
