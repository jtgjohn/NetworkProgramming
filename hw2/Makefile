CFLAGS = -g -Wall
FILES = hw2server.cpp


all: 
	clang++ $(CFLAGS) $(FILES) -o wordguess.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm wordguess.out