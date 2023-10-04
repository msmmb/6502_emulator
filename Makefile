CC = gcc
SRC = src/6502.c 
BIN = bin/run.o
CFLAGS = -D_6502_H -D_INS_H -g

build:
	$(CC) $(SRC) $(CFLAGS) -o $(BIN)
