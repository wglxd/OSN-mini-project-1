CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/prompt.c src/lexer.c src/parser.c src/builtins.c src/exec.c
OBJ = $(SRC:.c=.o)

shell: $(OBJ)
	$(CC) $(OBJ) -o shell

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) shell