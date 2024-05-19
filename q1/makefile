CC = g++
CFLAGS = -std=c++11 -Wall
SRC = tictactoe.cpp
EXEC = ttt

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(EXEC)
