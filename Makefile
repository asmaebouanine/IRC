CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98

SERVERSRS = server.cpp server_core.cpp handle_client.cpp handle_commads.cpp \
			Channel.cpp channelCommands.cpp commands.cpp

HEADERS = server_client.hpp Server.hpp Channel.hpp

NAME = ircserv

SERVEROBJS = $(SERVERSRS:.cpp=.o)

all: $(NAME)

$(NAME): $(SERVEROBJS)
	$(CC) $(CFLAGS) $(SERVEROBJS) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVEROBJS) 

fclean: clean
	rm -f $(NAME)

re: fclean all
