#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"

class Channel {
private:
	std::string _name;
	std::set<Client*> _members;
	std::set<Client*> _operators;
	std::set<Client*> _invited;
	std::string _topic;
	bool _inviteOnly;
	bool _topicRestricted;
	std::string _key;
	bool _hasKey;
	int _userLimit;

public:
	Channel();
	Channel(const std::string& name);
	Channel(const Channel& other);
	Channel& operator=(const Channel& other);
	~Channel();

	void addMember(Client* c);
	void removeMember(Client* c);
	bool isMember(Client* c) const;
	bool isEmpty() const;
	std::set<Client*> getMembers() const;

	void addOperator(Client* c);
	bool isOperator(Client* c) const;
	void removeOperator(Client* c);

	std::string getTopic() const;
	void setTopic(const std::string& topic);

	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	void setInviteOnly(bool v);
	void setTopicRestricted(bool v);

	std::string getKey() const;
	void setKey(const std::string& key);
	void clearKey();
	bool hasKey() const;

	int getUserLimit() const;
	void setUserLimit(int limit);
	void clearUserLimit();

	void invite(Client* c);
	bool isInvited(Client* c) const;
	void consumeInvite(Client* c);

	void broadcast(const std::string& msg, Client* exclude = 0);

	std::string getName() const;
};

#endif
