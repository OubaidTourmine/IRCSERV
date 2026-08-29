#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

#define SERVER_NAME "ft_ircserv"

// Numeric reply helper functions
inline std::string RPL_WELCOME(const std::string& nick) {
	return ":ft_ircserv 001 " + nick + " :Welcome to the Internet Relay Network " + nick;
}

inline std::string RPL_TOPIC(const std::string& nick, const std::string& chan, const std::string& topic) {
	return ":ft_ircserv 332 " + nick + " " + chan + " :" + topic;
}

inline std::string RPL_NAMREPLY(const std::string& nick, const std::string& chan, const std::string& names) {
	return ":ft_ircserv 353 " + nick + " = " + chan + " :" + names;
}

inline std::string RPL_ENDOFNAMES(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 366 " + nick + " " + chan + " :End of /NAMES list.";
}

inline std::string ERR_NEEDMOREPARAMS(const std::string& nick, const std::string& cmd) {
	return ":ft_ircserv 461 " + nick + " " + cmd + " :Not enough parameters";
}

inline std::string ERR_PASSWDMISMATCH(const std::string& nick) {
	return ":ft_ircserv 464 " + nick + " :Password incorrect";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 403 " + nick + " " + chan + " :No such channel";
}

inline std::string ERR_NOTONCHANNEL(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 442 " + nick + " " + chan + " :You're not on that channel";
}

inline std::string ERR_CHANOPRIVSNEEDED(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 482 " + nick + " " + chan + " :You're not channel operator";
}

inline std::string ERR_INVITEONLYCHAN(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 473 " + nick + " " + chan + " :Cannot join channel (+i)";
}

inline std::string ERR_CHANNELISFULL(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 471 " + nick + " " + chan + " :Cannot join channel (+l)";
}

inline std::string ERR_BADCHANNELKEY(const std::string& nick, const std::string& chan) {
	return ":ft_ircserv 475 " + nick + " " + chan + " :Cannot join channel (+k)";
}

inline std::string ERR_CANNOTSENDTOCHAN(const std::string& nick, const std::string& chan) {
    return ":ft_ircserv 404 " + nick + " " + chan + " :Cannot send to channel";
}

inline std::string ERR_NORECIPIENT(const std::string& nick, const std::string& cmd) {
    return ":ft_ircserv 411 " + nick + " :No recipient given (" + cmd + ")";
}

inline std::string ERR_NOTEXTTOSEND(const std::string& nick) {
    return ":ft_ircserv 412 " + nick + " :No text to send";
}

inline std::string ERR_NOTREGISTERED(const std::string& nick) {
    return ":ft_ircserv 451 " + nick + " :You have not registered";
}

inline std::string RPL_NOTOPIC(const std::string& nick, const std::string& chan) {
    return ":ft_ircserv 331 " + nick + " " + chan + " :No topic is set";
}

inline std::string ERR_NOSUCHNICK(const std::string& nick, const std::string& target) {
    return ":ft_ircserv 401 " + nick + " " + target + " :No such nick/channel";
}

#endif
