NAME = webserv

CC = c++

FLAGS = -fsanitize=address -g #-Wall -Wextra -Werror -std=c++98

SRC	=	./pars_config/config.cpp main.cpp ./AllServer/HttpServer.cpp ./Request/Request.cpp \
		./FunctionTools.cpp ./Request/Get.cpp ./Request/Post.cpp ./Request/Delete.cpp

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

# Define source directories for vpath
VPATH = ./pars_config:./AllServer:./Request

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: %.cpp ./pars_config/config.hpp  ./AllServer/HttpServer.hpp Request/Request.hpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME) zbi

re: fclean all

.PHONY: all clean fclean re

test:
	c++ -fsanitize=address -std=c++98 Request/z.cpp -o zbi && echo "\n" && ./zbi