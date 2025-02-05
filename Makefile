DCONF = ./config/pars_config/
DOBJ = ./obj/

OBJ = $(DOBJ)/config.o  $(DOBJ)/main.o

NAME = Webserv

FLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

CC = c++

all: $(DOBJ) ${NAME}

${NAME}: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $@

$(DOBJ)/%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

$(DOBJ)/%.o: $(DCONF)%.cpp $(DCONF)config.hpp 
	$(CC) $(FLAGS) -c $< -o $@

$(DOBJ):
	mkdir -p $(DOBJ)

clean:
	rm -rf $(DOBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all
