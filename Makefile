CC = gcc
CFLAGS = -Wall -Iinclude -g
LDFLAGS = -lm

SRC = src/main.c src/references.c src/constant_pool.c src/printer.c src/cp_resolver.c src/bytecodes.c src/methods.c src/attributes.c src/fields.c
OBJ = $(SRC:.c=.o)
EXEC = bin

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(EXEC)
