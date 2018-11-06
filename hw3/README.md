HW3 CRI Server
The server uses an IP6 server address. The socket option

USER
	USER works like specified in the assignment.
LIST
	LIST will list all members of a channel if the second argument is an existing channel.
	In all other cases it will just list all of the channels. (Including an invalid channel name)
JOIN
	JOIN will create a new channel if the name is valid and it does not exist.
	If the channel already exists it will add the user to it.
	Otherwise it will give an error message.
PART
	PART will remove the user from a channel if it is specified and they are in it.
	Otherwise if there is no argument it will remove them from all channels they are in.
	Otherwise it will send an error message.
OPERATOR
	OPERATOR will bestow operator status if the password given is correct. 
	If it is wrong or no argument is given it will send and error message.
	If no password is set it in the server it will allow no operators.
KICK
	KICK will only work with three arguments. It will only kick the specified 
	user if they are in the specified channel, otherwise it will send an error message.
	If user is not an operator, no one will be kicked.
PRIVMSG
	PRIVMSG works exaclty as specified in the assignment.
QUIT
	QUIT will remove the user from all channels they are in, and will notify
	all other users in those channels that the user quit (different error message than PART). 
	It will remove the user from the operator set and from usernames and clifds, and then
	disconnect the user.

Note: If a user disconnects without saying quit, the server will treat it as if they
have entered quit and handle it the exact same way.