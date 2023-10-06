CC = gcc
SRC = src/main.c src/6502.c
BIN = bin/run.o
CFLAGS = -g

all: build

build: 
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)
	
clean:
	rm -rf $(BIN)
