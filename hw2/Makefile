CFLAGS = -g -Wall
FILES = hw2server.cpp ../unpv13e/libunp.a


all: 
	clang++ $(CFLAGS) $(FILES) -o hw2server.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm hw2server.out