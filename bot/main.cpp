#include "Bot.hpp"

#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

static bool validPort(const std::string& value, int& port)
{
    if (value.empty())
        return false;
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(value[i]))
            return false;
    }
    long parsed = std::atol(value.c_str());
    if (parsed < 1024 || parsed > 65535)
        return false;
    port = static_cast<int>(parsed);
    return true;
}

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 6)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <host> <port> <password> [nick] [channel]" << std::endl;
        return 1;
    }

    int port = 0;
    if (!validPort(argv[2], port) || std::string(argv[3]).empty())
    {
        std::cerr << "Error: invalid port or empty password" << std::endl;
        return 1;
    }

    std::string nick;
    if (argc >= 5)
        nick = argv[4];
    else
        nick = "ircbot";

    std::string channel;
    if (argc == 6)
        channel = argv[5];
    else
        channel = "#bot";
    if (channel.empty() || (channel[0] != '#' && channel[0] != '&'))
        channel = "#" + channel;

    signal(SIGINT, Bot::signalHandler);
    signal(SIGQUIT, Bot::signalHandler);
    signal(SIGPIPE, SIG_IGN);

    try
    {
        Bot bot(argv[1], port, argv[3], nick, channel);
        bot.connectToServer();
        bot.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}