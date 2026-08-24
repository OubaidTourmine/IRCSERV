#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

struct command 
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

command ParseLine(const std::string &line);

#endif
