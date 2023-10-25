CC = gcc
SRC = main.c 6502.c
BIN = ./run.o
CFLAGS = -g

all: build

build: 
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)
	
clean:
	rm -rf $(BIN)
