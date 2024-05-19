CC = g++
CFLAGS = -std=c++11 -Wall
SRCS = tictactoe.cpp mynetcat.cpp
EXECS = ttt mync

all: $(EXECS)

ttt: tictactoe.cpp
	$(CC) $(CFLAGS) -o $@ $^

mync: mynetcat.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(EXECS)
