##
# d-lisp
#
# @file
# @version 0.1

NAME = d-lisp
SRC = src/parser.c
OBJ = $(SRC:.c=.o)

CFLAGS="-Wall"

all: $(NAME)

$(NAME): $(OBJ)
	gcc -o $(NAME) $(OBJ) $(CFLAGS)

# end
