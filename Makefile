NAME = ircserv

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
		srcs/commands/Quit.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
