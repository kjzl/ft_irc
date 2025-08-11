# Command Implementations and the IRC protocol

### Channels

Channels are a way to group users together in a chat room. They are identified by a name that starts with a `#` character.

For our Case, they are created with [JOIN](https://modern.ircdocs.horse/#join-message)

#### MODE

##### Channel modes

Operators can set modes on channels to control their behavior.
[modes](https://modern.ircdocs.horse/#channel-modes) are set with the [MODE](https://modern.ircdocs.horse/#mode-message) command by an operator.

##### examples:
/MODE #irssi -o+i mike

##### User Modes

[src](https://github.com/grawity/irc-docs/blob/master/misc/irc-documentation-jilles/client%20protocol.txt)
MODE can also set some things on the user. in out case, we implement being able to unset yourself as the operator:
$$ MODE <nickname> -o$$
and asking your mode status:
$$ MODE <nickname>$$

#### Limit
[l - Limit](https://www.afternet.org/help/irc/modes) save the maximum amount of clients allowed to be in a channel.
+l 30 for example, will limit the channel to 30 users.
-l will remove the limit.

Setting the user limit to 0 (zero) is NOT valid or meaningful by the protocol standard. According to the specifications and typical IRC server behavior:
    Setting +l 0 (limit 0) is usually rejected as invalid by most modern IRC servers.
    To disable the user limit, you should remove the +l mode entirely with:
/mode #channel -l

There is no definition in the protocol for a zero user limit, and servers interpret a missing or zero value as meaning “no limit is set” (i.e., unlimited, or as many as the server allows).
    Setting a limit of zero is interpreted as removing the limit in IRC (i.e., unlimited), not as setting the allowed users to zero.
    Attempting to set +l 0 may result in an error or simply be treated as if the limit is unset, depending on the server, but is not a valid way to restrict all users from joining.

### capabilities

	'#' //handling only public channels in general

#### commands

PASS: Authenticate with the server
NICK: Set your nickname
USER: Set your username and real name
JOIN: Join a channel
PRIVMSG: Send a private message to a user
NOTICE: Send a notice to a user (alias of PRIVMSG)

channel operator commands:
- KICK - Eject client from the channel
- INVITE - Invite client to a channel
- TOPIC - Change or view the channel topic
- MODE - Change the channel’s mode:
· i: Set/remove Invite-only channel
· t: Set/remove the restrictions of the TOPIC command to channel operators
· k: Set/remove the channel key (password) (requires an argument to set)
· o: Give/take channel operator privilege (requires an argument of valid nickname)
· l: Set/remove the user limit to channel (requires an argument of a number to set)
MODE also shows the users mode status:
$$ MODE <nickname>$$
MODE can also be used to show the channel modes:
$$ MODE #channel$$
user any of the above letters like this 

additional modes:
WHO: Show who is in the channel
