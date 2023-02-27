##
# d-lisp
#
# @file
# @version 0.1

NAME = d-lisp
SRC = src/main.c \
	src/lval.c \
	src/parser.c \
	src/mpc.c
OBJ = $(SRC:.c=.o)

CFLAGS = -iquote include -g -Wall -lm

all: $(NAME)

$(NAME): $(OBJ)
	gcc -o $(NAME) $(OBJ) $(CFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# end
