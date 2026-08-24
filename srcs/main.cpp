#include "Server.hpp"
#include <cstdlib>

static bool isValidPort(const std::string &portStr, int &portOut)
{
	if (portStr.empty())
		return false;

	for (size_t i = 0; i < portStr.length(); ++i)
	{
		if (!std::isdigit(portStr[i]))
			return false;
	}

	long val = std::atol(portStr.c_str());
	if (val < 1024 || val > 65535)
		return false;

	portOut = static_cast<int>(val);
	return true;
}

static bool isValidPassword(const std::string &password)
{
	if (password.empty())
		return false;
	for (size_t i = 0; i < password.length(); ++i)
	{
		if (std::isspace(password[i]))
			return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << RED << "Error: Invalid number of arguments." << WHI << std::endl;
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	int port = 0;
	if (!isValidPort(argv[1], port))
	{
		std::cerr << RED << "Error: Invalid port '" << argv[1] << "'." << WHI << std::endl;
		std::cerr << "Port must be an integer between 1024 and 65535." << std::endl;
		return 1;
	}

	std::string password = argv[2];
	if (!isValidPassword(password))
	{
		std::cerr << RED << "Error: Invalid password." << WHI << std::endl;
		std::cerr << "Password cannot be empty or contain whitespace." << std::endl;
		return 1;
	}

	std::cout << GRE << "Arguments parsed successfully!" << WHI << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Password: " << password << std::endl;

	signal(SIGINT, Server::SignalHandler);
	signal(SIGQUIT, Server::SignalHandler);
	signal(SIGPIPE, SIG_IGN);

	try
	{
		Server server;
		server.ServerInit(port, password);
		server.ServerRun();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Error: " << e.what() << WHI << std::endl;
		return 1;
	}

	std::cout << GRE << "Server closed successfully." << WHI << std::endl;
	return 0;
}
