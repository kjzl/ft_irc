# README

## Requirements

Executable will run like:
$$./ircserv <port> <password>$$

Let us use port 6667, as it is the [standard IRC port](https://datatracker.ietf.org/doc/html/rfc7194).

## Sources

- https://ircv3.net/irc/
- https://modern.ircdocs.horse/
- non-blocking: https://www.geeksforgeeks.org/c/non-blocking-io-with-pipes-in-c/
- https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

## Guides

- https://reactive.so/post/42-a-comprehensive-guide-to-ft_irc/
- https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9

figuring out how a server should respond:
$ nc irc.libera.chat 6667
NICK mynick
USER myident 0 * :My Name
then use  JOIN, PRIVMSG ....
