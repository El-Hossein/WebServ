# Explicitly list all .cpp files
SRC = ./pars_config/config.cpp main.cpp

# Define the object directory
OBJ_DIR = obj

# Generate the list of object files with the object directory
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

# Define source directories for vpath
VPATH = ./pars_config:.

# Define the output executable name
NAME = Webserv

# Compiler flags
FLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

# Compiler
CC = c++

# Default target
all: $(NAME)

# Rule to link the object files into the final executable
$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $@

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: %.cpp ./pars_config/config.hpp 
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

# Rule to clean up object files
clean:
	rm -rf $(OBJ_DIR)

# Rule to clean up object files and the executable
fclean: clean
	rm -rf $(NAME)

# Rule to recompile everything
re: fclean all

.PHONY: all clean fclean re
