#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

class Client {
private:
	int         _fd;
	std::string _nick;
	std::string _user;
	std::string _host;
	std::string _realname;
	std::string _ipAdd;
	std::string _buffer;
	bool        _registered;
	bool        _passOk;

public:
	Client();
	Client(int fd);
	Client(int fd, const std::string& nick, const std::string& user, const std::string& host);
	Client(const Client& other);
	Client& operator=(const Client& other);
	~Client();

	int getFd() const;
	int GetFd() const;
	void setFd(int fd);
	void SetFd(int fd);

	std::string getNick() const;
	void setNick(const std::string& nick);

	std::string getUser() const;
	void setUser(const std::string& user);
	void setUsername(const std::string& username);

	std::string getHost() const;
	void setHost(const std::string& host);

	std::string getRealname() const;
	void setRealname(const std::string& realname);

	std::string getIpAdd() const;
	void setIpAdd(const std::string& ipAdd);

	std::string prefix() const;

	bool isPassOk() const;
	bool IsPasswordAccepted() const;
	void setPassOk(bool passOk);

	bool isRegistered() const;
	void setRegistered(bool value);

	std::string& GetBuffer();
	void AppendToBuffer(const std::string& data);
	void ClearBuffer();

	std::set<std::string> _invitedChannels;

	std::string _outBuffer;
	bool        _quitting;

	void appendOutput(const std::string& msg);
	bool hasPendingOutput() const;
	bool flushOutput();
	bool isQuitting() const;
	void setQuitting(bool value);

	void queueOutput(const std::string& msg);
};

#endif
