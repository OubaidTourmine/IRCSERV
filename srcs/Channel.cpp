#include "Channel.hpp"

Channel::Channel()
{
	this->_name = "";
	this->_topic = "";
	this->_inviteOnly = false;
	this->_topicRestricted = true;
	this->_key = "";
	this->_hasKey = false;
	this->_userLimit = -1;
}

Channel::Channel(const std::string& name)
{
	this->_name = name;
	this->_topic = "";
	this->_inviteOnly = false;
	this->_topicRestricted = true;
	this->_key = "";
	this->_hasKey = false;
	this->_userLimit = -1;
}

Channel::Channel(const Channel& other)
{
	*this = other;
}

Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		this->_name = other._name;
		this->_members = other._members;
		this->_operators = other._operators;
		this->_invited = other._invited;
		this->_topic = other._topic;
		this->_inviteOnly = other._inviteOnly;
		this->_topicRestricted = other._topicRestricted;
		this->_key = other._key;
		this->_hasKey = other._hasKey;
		this->_userLimit = other._userLimit;
	}
	return *this;
}

Channel::~Channel() {}

// void Channel::addMember(Client* c)
// {
// 	if (c)
// 		this->_members.insert(c);
// }

// void Channel::removeMember(Client* c)
// {
// 	if (c)
// 	{
// 		this->_members.erase(c);
// 		this->_operators.erase(c);
// 		this->_invited.erase(c);
// 	}
// }

// bool Channel::isMember(Client* c) const
// {
// 	return (this->_members.find(c) != this->_members.end());
// }

// std::set<Client*> Channel::getMembers() const
// {
// 	return this->_members;
// }

// bool Channel::empty() const
// {
// 	return this->_members.empty();
// }

// void Channel::addOperator(Client* c)
// {
// 	if (c)
// 		this->_operators.insert(c);
// }

// void Channel::removeOperator(Client* c)
// {
// 	if (c)
// 		this->_operators.erase(c);
// }

// bool Channel::isOperator(Client* c) const
// {
// 	return (this->_operators.find(c) != this->_operators.end());
// }

// std::string Channel::getTopic() const
// {
// 	return this->_topic;
// }

// void Channel::setTopic(const std::string& topic)
// {
// 	this->_topic = topic;
// }

// bool Channel::isInviteOnly() const
// {
// 	return this->_inviteOnly;
// }

// bool Channel::isTopicRestricted() const
// {
// 	return this->_topicRestricted;
// }

// void Channel::setInviteOnly(bool v)
// {
// 	this->_inviteOnly = v;
// }

// void Channel::setTopicRestricted(bool v)
// {
// 	this->_topicRestricted = v;
// }

// std::string Channel::getKey() const
// {
// 	return this->_key;
// }

// void Channel::setKey(const std::string& key)
// {
// 	this->_key = key;
// 	this->_hasKey = true;
// }

// void Channel::clearKey()
// {
// 	this->_key = "";
// 	this->_hasKey = false;
// }

// bool Channel::hasKey() const
// {
// 	return this->_hasKey;
// }

// int Channel::getUserLimit() const
// {
// 	return this->_userLimit;
// }

// void Channel::setUserLimit(int limit)
// {
// 	this->_userLimit = limit;
// }

// void Channel::clearUserLimit()
// {
// 	this->_userLimit = -1;
// }

// void Channel::invite(Client* c)
// {
// 	if (c)
// 		this->_invited.insert(c);
// }

// bool Channel::isInvited(Client* c) const
// {
// 	return (this->_invited.find(c) != this->_invited.end());
// }

// void Channel::consumeInvite(Client* c)
// {
// 	if (c)
// 		this->_invited.erase(c);
// }

// void Channel::broadcast(const std::string& msg, Client* exclude)
// {
// 	for (std::set<Client*>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
// 	{
// 		if (*it != exclude)
// 			(*it)->queueOutput(msg);
// 	}
// }

std::string Channel::getName() const
{
	return this->_name;
}


void Channel::addMember(Client* cli)
{
	if (cli)
		this->_members.insert(cli);
}

void Channel::removeMember(Client* cli)
{
	if (cli)
	{
		this->_members.erase(cli);
		this->_operators.erase(cli);
		this->_invited.erase(cli);
	}
}

bool Channel::isMember(Client* cli) const
{
	return (this->_members.find(cli) != this->_members.end());
}

bool Channel::isEmpty() const
{
	return this->_members.empty();
}

std::set<Client*> Channel::getMembers() const
{
	return this->_members;
}

void Channel::addOperator(Client* cli)
{
	if (cli)
		this->_operators.insert(cli);
}

bool Channel::isOperator(Client* cli) const
{
	return (this->_operators.find(cli) != this->_operators.end());
}

void Channel::removeOperator(Client* cli)
{
	if (cli)
		this->_operators.erase(cli);
}

std::string Channel::getTopic() const
{
	return this->_topic;
}

void Channel::setTopic(const std::string& topic)
{
	this->_topic = topic;
}

bool Channel::isTopicRestricted() const
{
	return this->_topicRestricted;
}

void Channel::setTopicRestricted(bool v)
{
	this->_topicRestricted = v;
}

void Channel::broadcast(const std::string& msg, Client* exclude)
{
	for (std::set<Client*>::iterator it = this->_members.begin(); it != this->_members.end(); ++it)
	{
		if (*it != exclude)
			(*it)->queueOutput(msg);
	}
}

