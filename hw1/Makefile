CFLAGS = -g -Wall
FILES = tftp.cpp ../unpv13e/libunp.a


all: 
	clang++ $(CFLAGS) $(FILES) -o tftp.out

debug:
	clang++ $(CFLAGS) -D DEBUG_MODE $(FILES) -o debug

clean:
	rm tftp.out
