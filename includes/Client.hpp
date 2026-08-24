#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

class Client {
private:
	int _fd;
	std::string _nick;
	std::string _user;
	std::string _host;
	std::string _ipAdd;
	std::string _buffer;
	bool _registered;
	bool _passOk;

public:
	// Orthodox Canonical Form
	Client();
	Client(int fd);
	Client(int fd, const std::string& nick, const std::string& user, const std::string& host);
	Client(const Client& other);
	Client& operator=(const Client& other);
	~Client();

	// Identity & Getters/Setters
	int getFd() const;
	int GetFd() const;
	void setFd(int fd);
	void SetFd(int fd);

	std::string getNick() const;
	void setNick(const std::string& nick);

	std::string getUser() const;
	void setUser(const std::string& user);

	std::string getHost() const;
	void setHost(const std::string& host);

	std::string getIpAdd() const;
	void setIpAdd(const std::string& ipAdd);

	std::string prefix() const; // "nick!user@host"

	// Registration state
	bool isPassOk() const;
	void setPassOk(bool passOk);

	bool isRegistered() const;
	void setRegistered(bool value);

	// Buffer operations
	std::string& GetBuffer();
	void AppendToBuffer(const std::string& data);
	void ClearBuffer();

	// Operator / channel invite tracking
	std::set<std::string> _invitedChannels;

	// Output
	void queueOutput(const std::string& msg);
};

#endif
