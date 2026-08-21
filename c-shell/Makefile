CC = gcc
CFLAGS = -std=c23 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm -Iinclude
SRC = src/prompt.c src/lexer.c src/parser.c src/command.c src/builtins.c src/exec.c
OBJ = $(SRC:.c=.o)

all: shell.out

shell.out: $(OBJ)
	$(CC) $(OBJ) -o shell.out

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) shell.out