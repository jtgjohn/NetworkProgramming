CFLAGS = -g -Wall -std=c++11
FILES = hw3server.cpp


all:
	clang++ $(CFLAGS) $(FILES) -o hw3.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm hw3.out
