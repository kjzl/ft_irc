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

