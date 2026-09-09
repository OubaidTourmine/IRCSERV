#pragma once

#include <string>

#include "Command.hpp"

class Bot {
private:
    int         _fd;
    std::string _host;
    int         _port;
    std::string _password;
    std::string _nick;
    std::string _channel;

    std::string _inBuffer;
    std::string _outBuffer;
    bool        _registered;
    bool        _joined;
    bool        _running;
    static bool _signal;

    Bot(const Bot&);
    Bot& operator=(const Bot&);

public:
    Bot(const std::string& host, int port, const std::string& password,
        const std::string& nick, const std::string& channel);
    ~Bot();

    void connectToServer();
    void run();
    static void signalHandler(int);

private:
    void queueLine(const std::string& line);   
    void flushOutput();
    void readFromServer();
    void processBuffer();
    void handleLine(const std::string& line); 
    void sendRegistration();
    void reply(const std::string& target, const std::string& text);

    void handleBotCommand(const std::string& sender,
                          const std::string& target,
                          const std::string& text);
    void cmdHelp(const std::string& sender, const std::string& target);
    void cmdTime(const std::string& sender, const std::string& target);
    void cmdPing(const std::string& sender, const std::string& target);
};
