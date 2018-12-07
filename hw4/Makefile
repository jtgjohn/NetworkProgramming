CFLAGS = -g -Wall -lcrypto -std=c++11
FILES = dht.cpp


all:
	clang++ $(CFLAGS) $(FILES) -o hw4.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm hw4.out
