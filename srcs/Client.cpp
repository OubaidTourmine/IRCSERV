#include "Client.hpp"
#include <iostream>
#include <sys/socket.h>

Client::Client()
{
	this->_fd = -1;
	this->_nick = "";
	this->_user = "";
	this->_host = "";
	this->_realname = "";
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = false;
	this->_passOk = false;
	this->_quitting = false;
}

Client::Client(int fd)
{
	this->_fd = fd;
	this->_nick = "";
	this->_user = "";
	this->_host = "";
	this->_realname = "";
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = false;
	this->_passOk = false;
	this->_quitting = false;
}

Client::Client(int fd, const std::string& nick, const std::string& user, const std::string& host)
{
	this->_fd = fd;
	this->_nick = nick;
	this->_user = user;
	this->_host = host;
	this->_realname = "";
	this->_ipAdd = "";
	this->_buffer = "";
	this->_registered = true;
	this->_passOk = true;
	this->_quitting = false;
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
		this->_realname = other._realname;
		this->_ipAdd = other._ipAdd;
		this->_buffer = other._buffer;
		this->_registered = other._registered;
		this->_passOk = other._passOk;
		this->_invitedChannels = other._invitedChannels;
		this->_outBuffer = other._outBuffer;
		this->_quitting = other._quitting;
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

void Client::setUsername(const std::string& username)
{
	this->_user = username;
}

std::string Client::getHost() const
{
	return this->_host;
}

void Client::setHost(const std::string& host)
{
	this->_host = host;
}

std::string Client::getRealname() const
{
	return this->_realname;
}

void Client::setRealname(const std::string& realname)
{
	this->_realname = realname;
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
	std::string host_or_ip;
	if (this->_host.empty())
		host_or_ip = this->_ipAdd;
	else
		host_or_ip = this->_host;
	return this->_nick + "!" + this->_user + "@" + host_or_ip;
}

bool Client::isPassOk() const
{
	return this->_passOk;
}

bool Client::IsPasswordAccepted() const
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

bool Client::isQuitting() const
{
	return this->_quitting;
}

void Client::setQuitting(bool value)
{
	this->_quitting = value;
}

void Client::appendOutput(const std::string& msg)
{
	std::string output = msg;
	if (output.size() < 2 || output.substr(output.size() - 2) != "\r\n")
		output += "\r\n";
	this->_outBuffer += output;
}

bool Client::hasPendingOutput() const
{
	return !this->_outBuffer.empty();
}

bool Client::flushOutput()
{
	if (this->_outBuffer.empty())
		return true;
	ssize_t sent = send(this->_fd, this->_outBuffer.c_str(), this->_outBuffer.size(), 0);
	if (sent > 0)
	{
		this->_outBuffer.erase(0, static_cast<size_t>(sent));
		return true;
	}
	return sent == -1 ? false : true;
}

void Client::queueOutput(const std::string& msg)
{
	appendOutput(msg);
}
