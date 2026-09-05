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

    std::string _inBuffer;      // accumulates recv()'d bytes, drained on '\n'
    std::string _outBuffer;     // pending writes, drained on POLLOUT
    bool        _registered;    // set when 001 arrives
    bool        _joined;        // set when our own JOIN echoes back
    bool        _running;
    static bool _signal;        // set by SIGINT handler

    // non-copyable: private, undefined  (C++98 idiom)
    Bot(const Bot&);
    Bot& operator=(const Bot&);

public:
    Bot(const std::string& host, int port, const std::string& password,
        const std::string& nick, const std::string& channel);
    ~Bot();

    void connectToServer();     // socket + connect + O_NONBLOCK
    void run();                 // the poll loop
    static void signalHandler(int);

private:
    void queueLine(const std::string& line);   // append to _outBuffer (+ CRLF)
    void flushOutput();                        // POLLOUT: partial-aware send
    void readFromServer();                     // POLLIN: recv + append
    void processBuffer();                      // drain complete lines
    void handleLine(const std::string& line);  // ParseLine + dispatch
    void sendRegistration();
    void reply(const std::string& target, const std::string& text);

    // BotCommands.cpp
    void handleBotCommand(const std::string& sender,
                          const std::string& target,
                          const std::string& text);
    void cmdHelp(const std::string& sender, const std::string& target);
    void cmdTime(const std::string& sender, const std::string& target);
    void cmdPing(const std::string& sender, const std::string& target);
};
