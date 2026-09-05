#include "Server.hpp"
#include <sstream>
#include <cstdlib>

static std::string intToString(int val)
{
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

void Server::HandleMode(Client &client, const command &cmd)
{
	std::string clientNick;
	if (client.getNick().empty())
		clientNick = "*";
	else
		clientNick = client.getNick();
	if (!client.isRegistered())
	{
		SendReply(client.GetFd(), ERR_NOTREGISTERED(clientNick));
		return;
	}

	if (cmd.params.empty())
	{
		SendReply(client.GetFd(), ERR_NEEDMOREPARAMS(client.getNick(), "MODE"));
		return;
	}

	std::string target = cmd.params[0];
	if (target[0] != '#' && target[0] != '&')
		return; // We only handle channel modes for now

	Channel* chan = GetChannelByName(target);
	if (!chan)
	{
		SendReply(client.GetFd(), ERR_NOSUCHCHANNEL(client.getNick(), target));
		return;
	}

	if (cmd.params.size() == 1)
	{
		std::string modes = "+";
		std::string params = "";
		
		if (chan->isInviteOnly()) modes += "i";
		if (chan->isTopicRestricted()) modes += "t";
		if (chan->hasKey()) {
			modes += "k";
			params += chan->getKey() + " ";
		}
		if (chan->getUserLimit() != -1) {
			modes += "l";
			params += intToString(chan->getUserLimit()) + " ";
		}
		
		if (params.length() > 0 && params[params.length()-1] == ' ')
			params.erase(params.length()-1);

		SendReply(client.GetFd(), RPL_CHANNELMODEIS(client.getNick(), chan->getName(), modes, params));
		return;
	}

	if (!chan->isOperator(&client))
	{
		SendReply(client.GetFd(), ERR_CHANOPRIVSNEEDED(client.getNick(), chan->getName()));
		return;
	}

	std::string modestring = cmd.params[1];
	bool adding = true;
	size_t paramIndex = 2;

	std::string modesChanged = "";
	std::string paramsChanged = "";

	for (size_t i = 0; i < modestring.size(); ++i)
	{
		char m = modestring[i];
		if (m == '+') { adding = true; if (modesChanged.empty() || modesChanged[modesChanged.length()-1] != '+') modesChanged += "+"; }
		else if (m == '-') { adding = false; if (modesChanged.empty() || modesChanged[modesChanged.length()-1] != '-') modesChanged += "-"; }
		else if (m == 'i')
		{
			if (chan->isInviteOnly() != adding)
			{
				chan->setInviteOnly(adding);
				modesChanged += "i";
			}
		}
		else if (m == 't')
		{
			if (chan->isTopicRestricted() != adding)
			{
				chan->setTopicRestricted(adding);
				modesChanged += "t";
			}
		}
		else if (m == 'k')
		{
			if (adding)
			{
				if (paramIndex < cmd.params.size())
				{
					std::string key = cmd.params[paramIndex++];
					chan->setKey(key);
					modesChanged += "k";
					paramsChanged += " " + key;
				}
			}
			else
			{
                if (paramIndex < cmd.params.size()) {
                    std::string key = cmd.params[paramIndex++];
                    if (chan->hasKey() && chan->getKey() == key) {
                        chan->clearKey();
                        modesChanged += "k";
                        paramsChanged += " " + key;
                    }
                } else if (chan->hasKey()) {
                    chan->clearKey();
                    modesChanged += "k";
                }
			}
		}
		else if (m == 'o')
		{
			if (paramIndex < cmd.params.size())
			{
				std::string targetNick = cmd.params[paramIndex++];
				Client* targetCli = GetClientByNick(targetNick);
				if (!targetCli || !chan->isMember(targetCli))
				{
					SendReply(client.GetFd(), ERR_USERNOTINCHANNEL(client.getNick(), targetNick, chan->getName()));
				}
				else
				{
					if (adding && !chan->isOperator(targetCli))
					{
						chan->addOperator(targetCli);
						modesChanged += "o";
						paramsChanged += " " + targetNick;
					}
					else if (!adding && chan->isOperator(targetCli))
					{
						chan->removeOperator(targetCli);
						modesChanged += "o";
						paramsChanged += " " + targetNick;
					}
				}
			}
		}
		else if (m == 'l')
		{
			if (adding)
			{
				if (paramIndex < cmd.params.size())
				{
					std::string limitStr = cmd.params[paramIndex++];
					int limit = std::atoi(limitStr.c_str());
					if (limit > 0)
					{
						chan->setUserLimit(limit);
						modesChanged += "l";
						paramsChanged += " " + limitStr;
					}
				}
			}
			else
			{
				if (chan->getUserLimit() != -1)
				{
					chan->clearUserLimit();
					modesChanged += "l";
				}
			}
		}
		else
		{
			SendReply(client.GetFd(), ERR_UNKNOWNMODE(client.getNick(), m, chan->getName()));
		}
	}

    std::string cleanModes = "";
    char lastSign = ' ';
    for (size_t i = 0; i < modesChanged.size(); i++) {
        if (modesChanged[i] == '+' || modesChanged[i] == '-') {
            lastSign = modesChanged[i];
            if (i == modesChanged.size() - 1) continue;
            bool hasMode = false;
            for (size_t j = i + 1; j < modesChanged.size(); j++) {
                if (modesChanged[j] != '+' && modesChanged[j] != '-') {
                    hasMode = true;
                    break;
                }
            }
            if (hasMode) {
                if (cleanModes.empty() || cleanModes[cleanModes.length()-1] != lastSign) {
                    cleanModes += lastSign;
                }
            }
        } else {
            cleanModes += modesChanged[i];
        }
    }
    
	while (cleanModes.size() > 0 && (cleanModes[cleanModes.length()-1] == '+' || cleanModes[cleanModes.length()-1] == '-'))
	{
		cleanModes.erase(cleanModes.length()-1);
	}

	if (!cleanModes.empty())
	{
		std::string msg = ":" + client.prefix() + " MODE " + chan->getName() + " " + cleanModes + paramsChanged;
		chan->broadcast(msg);
	}
}
