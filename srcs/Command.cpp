#include "Command.hpp"
#include <cstddef>

static size_t skipWhiteSpaces(const std::string &s, size_t i)
{
	while (i < s.size() && s[i] == ' ')
		++i;
	return i;
}

static std::string readToSpace(const std::string &s, size_t &i)
{
	size_t start = i;
	while (i < s.size() && s[i] != ' ')
		++i;
	return s.substr(start, i - start);
}

command ParseLine(const std::string &line)
{
	command cmd;
	size_t i = 0;

	if (line.empty())
		return cmd;

	// Step 1: Prefix (optional, starts with ':')
	if (line[i] == ':')
	{
		i++;
		cmd.prefix = readToSpace(line, i);
		i = skipWhiteSpaces(line, i);
	}

	// Step 2: Command
	cmd.command = readToSpace(line, i);

	// Step 3: Parameters (max 15 per RFC 1459)
	while (cmd.params.size() < 15)
	{
		i = skipWhiteSpaces(line, i);
		if (i >= line.size())
			break;
		if (line[i] == ':')
		{
			cmd.params.push_back(line.substr(i + 1));
			break;
		}
		cmd.params.push_back(readToSpace(line, i));
	}
	return cmd;
}
