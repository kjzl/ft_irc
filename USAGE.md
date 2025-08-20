
# Usage Guide

This guide explains how to interact with the **ft_irc** server using the IRC protocol, with practical examples for each command and a summary of special features and behaviors.

---

## Connecting to the Server

You can use any IRC client or even `nc` (netcat) for manual inputs.

**Example (with netcat):**
```sh
nc localhost 6667
```
Replace `localhost` and `6667` with your server address and the port you started ircserv with.

---

## Basic Connection Sequence

A typical session starts with authenticating and registering:

```irc
PASS mysecretpass
NICK alice
USER alice 0 * :Alice Example
```
- `PASS` – Authenticate with the server (use the server password)
- `NICK` – Set your nickname (has to be unique across the server)
- `USER` – Set your username and real name

---

## Supported IRC Commands

### 1. JOIN

Join a channel (channels always start the channelname with `#`):

```irc
JOIN #mychannel
```
- If the channel does not exist, it will be created.
- If you create a channel, you become the operator of that channel.

---

### 2. PRIVMSG

Send a private message to a user or a channel:

```irc
PRIVMSG #mychannel :Hello, everyone!
PRIVMSG bob :Hello, Bob!
```

---

### 3. NOTICE

[NOTICE](https://www.irchelp.org/protocol/notice.html) is used for automatic or informational messages, such as system notices or response from automated processes. According to the IRC specification, automatic replies MUST NEVER be sent in response to a NOTICE message. This convention is intended to prevent automated message loops between clients or bots.
Send a notice to a user (or channel). Other than that, it should behave very similar to PRIVMSG.

```irc
NOTICE #mychannel :This is an important notice.
NOTICE carol :You have a new message.
```

---

### 4. QUIT

disconnect from the server (optionally with a quit message)

```irc
QUIT :going outside
```

---

### 5. WHO

List users in a channel:

```irc
WHO #mychannel
```

---

### 6. KICK

(Operator only) Eject a client from the channel (with an optional reason as the last argument):

```irc
KICK #mychannel bob :Spamming not allowed
```

---

### 7. INVITE

(Operator only) Invite a user to a channel (especially meaningful if +i invite-only):

```irc
INVITE dave #mychannel
```

---

### 8. TOPIC

Set or view the channel topic:

```irc
TOPIC #mychannel :Welcome to our awesome channel!
TOPIC #mychannel
```
- Without a message, shows the current topic.

---

### 9. MODE

Change channel or user modes (operator only for channel modes):

#### Channel Modes

```irc
MODE #mychannel +i           ; Set invite-only
MODE #mychannel -i           ; Remove invite-only
MODE #mychannel +t           ; Only ops can change topic
MODE #mychannel +k secret    ; Set channel key (password)
MODE #mychannel -k           ; Remove channel key
MODE #mychannel +o alice     ; Give operator status to alice
MODE #mychannel -o bob       ; Remove operator status from bob
MODE #mychannel +l 30        ; Set user limit to 30
MODE #mychannel -l           ; Remove user limit
MODE #mychannel              ; Show current channel modes
```

#### User Modes

```irc
MODE alice                   ; Show your mode status
```

> **Note:**  
> Setting the user limit to 0 (`+l 0`) is recommended.  
> To remove a user limit, always use `-l` (see below).

---

## Channel Behavior and Limitations

- Only public channels (`#`) are supported.
- Channels are created on demand with the `JOIN` command.
- Channel modes control access and privileges:
  - `i`: Invite-only
  - `t`: Only ops may change topic
  - `k`: Channel password (key)
  - `o`: Operator privilege
  - `l`: User limit

**User limit (`+l`) details:**
- `+l <number>`: Limit channel to `<number>` users (e.g., `+l 30`)
- `-l`: Remove the user limit
- `+l 0` is the same as `-l`

---

## Example Session

```irc
PASS mysecretpass
NICK alice
USER alice 0 * :Alice Example
JOIN #backyard
PRIVMSG #backyard :Hello all!
MODE #backyard +i
INVITE bob #backyard
KICK #backyard carol :You are obviously a cat.
TOPIC #backyard :discuss the important stuff.
MODE #backyard +l 10
MODE #backyard -l
QUIT :gone to have lunch
```

---

## Further Reading

- [Command Implementations and the IRC protocol](./COMMANDS.md)
- [Modern IRC Documentation](https://modern.ircdocs.horse/)
- [IRCv3 Specifications](https://ircv3.net/irc/)

---

For troubleshooting and advanced configuration, see [docs/](./docs/).
