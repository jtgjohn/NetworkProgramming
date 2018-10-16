John Gay
Andrew Leaf

Network Programming Fall 2018 Homework 2


This homework builds a server that we can netcat into.
The server can hold up to 5 clients at one time. 
When a client is connected, they are told how many other clients 
are currently playing the game and how many letters are in the secret word.
When a client makes a guess, if it has the wrong number of letters, an error
message is printed to that client only. If it has the correct number of letters,
the information of the guess is printed out to all clients connected. If 
the correct word is guessed, all clients are informed of who guessed the secret
word and what it was. All clients are then disconnected, and the server 
remains open and picks a new word.

The server prints the secret word to standard output. This is to make 
it much easier for the grader to see if it functions correclty.