NAME = ircserv
BOT_NAME = ircbot

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes

SRCS =	srcs/main.cpp \
		srcs/Client.cpp \
		srcs/Server.cpp \
		srcs/Command.cpp \
		srcs/Channel.cpp \
		srcs/commands/Pass.cpp \
		srcs/commands/Kick.cpp \
		srcs/commands/Nick.cpp \
		srcs/commands/User.cpp \
		srcs/commands/Join.cpp \
		srcs/commands/Privmsg.cpp \
		srcs/commands/Topic.cpp \
		srcs/commands/Ping.cpp \
		srcs/commands/Cap.cpp \
		srcs/commands/Part.cpp \
		srcs/commands/Quit.cpp \
		srcs/commands/Invite.cpp \
		srcs/commands/Mode.cpp

OBJS = $(SRCS:.cpp=.o)

BOT_SRCS = bot/main.cpp \
		srcs/Bot.cpp \
		srcs/Command.cpp

BOT_OBJS = $(BOT_SRCS:.cpp=.o)

all: $(NAME)

bonus: $(BOT_NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(BOT_NAME): $(BOT_OBJS)
	$(CXX) $(CXXFLAGS) $(BOT_OBJS) -o $(BOT_NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BOT_OBJS)

fclean: clean
	rm -f $(NAME) $(BOT_NAME)

re: fclean all

.PHONY: all bonus clean fclean re