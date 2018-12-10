CFLAGS = -g -Wall -lcrypto -std=c++11 -g
FILES = dht.cpp


all:
	clang++ $(CFLAGS) $(FILES) -o dht.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm dht.out
