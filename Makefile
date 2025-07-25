NAME = webserv

CC = c++

FLAGS = -fsanitize=address -g #-Wall -Wextra -Werror -std=c++98

SRC	=	./pars_config/config.cpp main.cpp ./AllServer/HttpServer.cpp \
		./FunctionTools.cpp ./Request/Request.cpp ./Request/Post.cpp \
		./cgi/cgiMain.cpp ./cgi/cgiExecPars.cpp ./cgi/cgiResponse.cpp \
		./Response/responseMain.cpp

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

# Define source directories for vpath
VPATH = ./pars_config:./AllServer:./Request:./cgi:./Response

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: %.cpp ./pars_config/config.hpp  ./AllServer/HttpServer.hpp Request/Request.hpp ./Response/responseHeader.hpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME) test

re: fclean all

.PHONY: all clean fclean re

test:
	c++ -fsanitize=address -std=c++98 Request/z.cpp -o test && echo "\n" && ./test