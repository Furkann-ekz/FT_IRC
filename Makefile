NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = src/main.cpp src/Server.cpp src/Client.cpp src/Channel.cpp src/Commands.cpp
OBJ = $(SRC:.cpp=.o)
INC = -Iinclude

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(INC) -o $(NAME) $(OBJ)
	rm -rf $(OBJ)

clean:
	rm -f valgrind.log
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f valgrind.log

re: fclean all

.PHONY: all clean fclean re