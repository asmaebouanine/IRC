
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

SERVERSRS = server.cpp server_core.cpp handle_client.cpp handle_commads.cpp \
            Channel.cpp channelCommands.cpp
SERVEROBJS = $(SERVERSRS:.cpp=.o)
NAME = ircserv

BOTSRCS = bot_bonus.cpp
BOTOBJS = $(BOTSRCS:.cpp=.o)
BOTNAME = irc_bot

HEADERS = server_client.hpp Server.hpp Channel.hpp

all: $(NAME)

$(NAME): $(SERVEROBJS)
	$(CC) $(CFLAGS) $(SERVEROBJS) -o $(NAME)

bonus: $(BOTNAME)

$(BOTNAME): $(BOTOBJS)
	$(CC) $(CFLAGS) $(BOTOBJS) -o $(BOTNAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVEROBJS) $(BOTOBJS)

fclean: clean
	rm -f $(NAME) $(BOTNAME)

re: fclean all
