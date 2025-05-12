NAME = ircserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = src/main.cpp src/Server.cpp src/Client.cpp src/Channel.cpp src/Commands.cpp
OBJS = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(SRCS) $(OBJS)
	c++ $(CFLAGS) $(SRCS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log ./ircserv 6667 mypass
