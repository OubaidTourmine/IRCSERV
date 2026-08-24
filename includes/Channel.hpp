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
	int _userLimit; // -1 means no limit

public:
	Channel();
	Channel(const std::string& name);
	Channel(const Channel& other);
	Channel& operator=(const Channel& other);
	~Channel();

	// Membership
	void addMember(Client* c);
	void removeMember(Client* c);
	bool isMember(Client* c) const;
	std::set<Client*> getMembers() const;
	bool empty() const;

	// Operators
	void addOperator(Client* c);
	void removeOperator(Client* c);
	bool isOperator(Client* c) const;

	// Topic
	std::string getTopic() const;
	void setTopic(const std::string& topic);

	// Modes
	bool isInviteOnly() const;      // +i
	bool isTopicRestricted() const; // +t
	void setInviteOnly(bool v);
	void setTopicRestricted(bool v);

	std::string getKey() const;     // +k
	void setKey(const std::string& key);
	void clearKey();
	bool hasKey() const;

	int getUserLimit() const;       // +l, -1 = unset
	void setUserLimit(int limit);
	void clearUserLimit();

	// Invites
	void invite(Client* c);
	bool isInvited(Client* c) const;
	void consumeInvite(Client* c);

	// Fan-out
	void broadcast(const std::string& msg, Client* exclude = 0);

	std::string getName() const;
};

#endif
