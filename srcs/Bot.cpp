#include "Bot.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <ctime>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

bool Bot::_signal = false;

Bot::Bot(const std::string& host, int port, const std::string& password,
		 const std::string& nick, const std::string& channel)
	: _fd(-1), _host(host), _port(port), _password(password), _nick(nick),
	  _channel(channel), _registered(false), _joined(false), _running(true)
{
}

Bot::~Bot()
{
	if (_fd != -1)
		close(_fd);
}

void Bot::signalHandler(int signum)
{
	(void)signum;
	Bot::_signal = true;
}

void Bot::connectToServer()
{
	struct sockaddr_in address;
	struct hostent *hostEntry;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw std::runtime_error("failed to create bot socket");

	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(_port);
	address.sin_addr.s_addr = inet_addr(_host.c_str());
	if (address.sin_addr.s_addr == INADDR_NONE)
	{
		hostEntry = gethostbyname(_host.c_str());
		if (!hostEntry)
			throw std::runtime_error("cannot resolve bot host '" + _host + "'");
		std::memcpy(&address.sin_addr, hostEntry->h_addr, hostEntry->h_length);
	}

	if (connect(_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
		throw std::runtime_error("cannot connect to " + _host);

	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to set bot socket non-blocking");

	sendRegistration();
}

void Bot::queueLine(const std::string& line)
{
	_outBuffer += line;
	if (line.size() < 2 || line.substr(line.size() - 2) != "\r\n")
		_outBuffer += "\r\n";
}

void Bot::flushOutput()
{
	if (_outBuffer.empty())
		return;

	ssize_t sent = send(_fd, _outBuffer.c_str(), _outBuffer.size(), 0);
	if (sent <= 0)
	{
		_running = false;
		return;
	}
	_outBuffer.erase(0, static_cast<size_t>(sent));
}

void Bot::readFromServer()
{
	char buffer[1024];
	ssize_t received = recv(_fd, buffer, sizeof(buffer), 0);

	if (received <= 0)
	{
		_running = false;
		return;
	}
	_inBuffer.append(buffer, static_cast<size_t>(received));
	processBuffer();
}

void Bot::processBuffer()
{
	while (true)
	{
		size_t end = _inBuffer.find('\n');
		if (end == std::string::npos)
			return;

		std::string line = _inBuffer.substr(0, end);
		_inBuffer.erase(0, end + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty())
			handleLine(line);
	}
}

void Bot::sendRegistration()
{
	queueLine("PASS " + _password);
	queueLine("NICK " + _nick);
	queueLine("USER " + _nick + " 0 * :IRC Bot");
}

static std::string prefixNick(const std::string& prefix)
{
	size_t bang = prefix.find('!');
	if (bang == std::string::npos)
		return prefix;
	return prefix.substr(0, bang);
}

void Bot::handleLine(const std::string& line)
{
	command parsed = ParseLine(line);
	for (size_t i = 0; i < parsed.command.size(); ++i)
		parsed.command[i] = static_cast<char>(std::toupper(parsed.command[i]));

	if (parsed.command == "PING")
	{
		if (!parsed.params.empty())
			queueLine("PONG :" + parsed.params[0]);
		return;
	}
	if (parsed.command == "001")
	{
		_registered = true;
		queueLine("JOIN " + _channel);
		return;
	}
	if (parsed.command == "433")
	{
		if (_nick.size() < 9)
			_nick += "_";
		else
			_nick = _nick.substr(0, 8) + "_";
		queueLine("NICK " + _nick);
		return;
	}
	if (parsed.command == "464")
	{
		std::cerr << "Bot authentication failed: wrong password" << std::endl;
		_running = false;
		return;
	}
	if (parsed.command == "ERROR")
	{
		_running = false;
		return;
	}
	if (parsed.command == "JOIN")
	{
		if (prefixNick(parsed.prefix) == _nick)
			_joined = true;
		return;
	}
	if (parsed.command == "PRIVMSG" && parsed.params.size() >= 2 && _registered)
	{
		std::string sender = prefixNick(parsed.prefix);
		if (sender != _nick)
			handleBotCommand(sender, parsed.params[0], parsed.params[1]);
	}
}

void Bot::reply(const std::string& target, const std::string& text)
{
	queueLine("PRIVMSG " + target + " :" + text);
}

void Bot::run()
{
	while (_running && !_signal)
	{
		struct pollfd descriptor;
		descriptor.fd = _fd;
		descriptor.events = POLLIN;
		if (!_outBuffer.empty())
			descriptor.events |= POLLOUT;
		descriptor.revents = 0;

		int result = poll(&descriptor, 1, -1);
		if (result == -1)
		{
			if (_signal)
				break;
			throw std::runtime_error("bot poll() failed");
		}
		if (descriptor.revents & (POLLHUP | POLLERR | POLLNVAL))
			break;
		if (descriptor.revents & POLLIN)
			readFromServer();
		if (descriptor.revents & POLLOUT)
			flushOutput();
	}

	if (_fd != -1 && _running)
	{
		queueLine("QUIT :Bot shutting down");
		while (!_outBuffer.empty())
		{
			struct pollfd descriptor;
			descriptor.fd = _fd;
			descriptor.events = POLLOUT;
			descriptor.revents = 0;
			if (poll(&descriptor, 1, 1000) <= 0)
				break;
			if (descriptor.revents & POLLOUT)
				flushOutput();
			if (descriptor.revents & (POLLHUP | POLLERR | POLLNVAL))
				break;
		}
	}
}

void Bot::handleBotCommand(const std::string& sender, const std::string& target,
						   const std::string& text)
{
	if (text.empty() || text[0] != '!')
		return;

	std::string replyTarget = target;
	if (target[0] != '#' && target[0] != '&')
		replyTarget = sender;

	size_t end = text.find(' ');
	size_t length;
	if (end == std::string::npos)
		length = std::string::npos;
	else
		length = end - 1;
	std::string name = text.substr(1, length);
	for (size_t i = 0; i < name.size(); ++i)
		name[i] = static_cast<char>(std::tolower(name[i]));

	if (name == "help")
		cmdHelp(sender, replyTarget);
	else if (name == "time")
		cmdTime(sender, replyTarget);
	else if (name == "ping")
		cmdPing(sender, replyTarget);
	else
		reply(replyTarget, "Unknown command. Try !help");
}

void Bot::cmdHelp(const std::string& sender, const std::string& target)
{
	(void)sender;
	reply(target, "Commands: !help, !time, !ping");
}

void Bot::cmdTime(const std::string& sender, const std::string& target)
{
	(void)sender;
	char formatted[32];
	std::time_t now = std::time(NULL);
	std::tm *local = std::localtime(&now);
	if (local && std::strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", local))
		reply(target, std::string("Current time: ") + formatted);
}

void Bot::cmdPing(const std::string& sender, const std::string& target)
{
	(void)sender;
	reply(target, "Pong!");
}
