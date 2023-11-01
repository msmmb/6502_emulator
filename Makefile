CC = gcc
SRC = src/main.c src/6502.c
BIN = bin/test.o
CFLAGS = -g

test: 
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)
	
clean:
	rm -rf bin
