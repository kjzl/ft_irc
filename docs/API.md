# API Reference

This document provides a comprehensive API reference for the core classes in the ft_irc project.

## Table of Contents

- [Server Class](#server-class)
- [Client Class](#client-class)
- [Channel Class](#channel-class)
- [Message Class](#message-class)
- [Command Classes](#command-classes)
- [Utility Classes](#utility-classes)
- [Error Codes](#error-codes)

## Server Class

**File**: `include/Server.hpp`, `src/Server.cpp`

The main server class that manages the IRC server lifecycle, client connections, and message routing.

### Constructor

```cpp
Server(int port, std::string password);
```

**Parameters**:
- `port`: TCP port to listen on (typically 6667 for IRC)
- `password`: Server password required for client authentication

**Example**:
```cpp
Server server(6667, "secretpassword");
```

### Public Methods

#### Core Operations

```cpp
void waitForRequests();
```
Main event loop that handles incoming connections and messages. Runs until server shutdown.

```cpp
void serverShutdown();
```
Gracefully shuts down the server, cleaning up resources and closing connections.

#### Client Management

```cpp
std::vector<Client>& getClients();
```
Returns reference to the client list for iteration and management.

```cpp
bool clientNickExists(CaseMappedString& toCheck);
```
Checks if a nickname is already in use by another client.

**Parameters**:
- `toCheck`: Nickname to verify (case-insensitive IRC comparison)

**Returns**: `true` if nickname exists, `false` otherwise

#### Channel Management

```cpp
std::map<std::string, Channel>& getChannels();
```
Returns reference to the channel map for access and management.

```cpp
Channel* mapChannel(const std::string& channelName);
```
Retrieves or creates a channel by name.

**Parameters**:
- `channelName`: Name of the channel (must start with '#')

**Returns**: Pointer to Channel object, or NULL if invalid name

#### Message Broadcasting

```cpp
void broadcastMsg(const Message &message) const;
```
Sends a message to all connected clients.

```cpp
void broadcastErrorMessage(MessageType type, std::vector<std::string>& args) const;
```
Broadcasts an error message to all clients.

```cpp
void quitClient(const Client &quitter, std::vector<std::string> &messageParams);
```
Handles client disconnection, notifying relevant channels and cleaning up state.

#### Getters

```cpp
const std::string& getName() const;
```
Returns the server name (defined as `HOSTNAME` in Server.hpp).

```cpp
const std::string& getPassword() const;
```
Returns the server password.

```cpp
const char* getTimeCreatedHumanReadable() const;
```
Returns human-readable server creation timestamp.

### Private Methods

```cpp
void serverInit();
void createListeningSocket();
void acceptConnection();
void processMessage(Client &client);
void removeClient(int pollIndexToRemove);
```

### Static Members

```cpp
static bool running_;
```
Global flag controlling server execution state.

## Client Class

**File**: `include/Client.hpp`, `src/Client.cpp`

Represents an individual IRC client connection with authentication state and user information.

### Constructors

```cpp
Client();
Client(const Client &other);
Client& operator=(const Client &other);
```

### Authentication & Registration

```cpp
void incrementRegistrationLevel();
```
Advances client through IRC registration process (PASS → NICK → USER).

```cpp
int getRegistrationLevel() const;
```
Returns current registration level (0-2, where 2 means fully authenticated).

```cpp
bool isAuthenticated() const;
```
Returns `true` if client has completed full IRC registration sequence.

### User Information

```cpp
const std::string& getNickname() const;
void setNickname(const std::string &nickname);
```
Get/set client nickname (must be unique on server).

```cpp
const std::string& getUsername() const;
void setUsername(const std::string &username);
```
Get/set client username (from USER command).

```cpp
const std::string& getRealname() const;
void setRealname(const std::string &realname);
```
Get/set client real name (from USER command).

### Socket Management

```cpp
int getSocket() const;
void setSocket(int socket);
```
Get/set client socket file descriptor.

### Message Handling

```cpp
const std::string& getRawMessage() const;
void setRawMessage(const std::string &rawMessage);
void appendRawMessage(const char partialMessage[BUFSIZ], size_t length);
void clearMessage();
```
Message buffer management for handling partial IRC messages.

### Message Sending

```cpp
void sendMessage(Message toSend) const;
```
Sends a formatted IRC message to this client.

```cpp
void sendErrorMessage(MessageType type, std::vector<std::string>& args) const;
```
Sends an IRC error response to this client.

```cpp
bool sendMessageTo(Message msg, const std::string recipientNickname, Server &server) const;
```
Sends a message to another client by nickname.

**Returns**: `false` if recipient not found, `true` if sent successfully

```cpp
void sendToFd(const std::string &string, int fd) const;
void sendMessageToFd(Message msg, int fd) const;
```
Low-level message sending to specific file descriptors.

### Operators

```cpp
bool operator==(const std::string nickname);
bool operator==(const Client &other);
```
Comparison operators for nickname matching and client equality.

## Channel Class

**File**: `include/Channel.hpp`, `src/Channel.cpp`

Manages IRC channels including membership, permissions, and modes.

### Constructors

```cpp
Channel(const std::string& name);
Channel(const Channel& other);
Channel& operator=(const Channel& other);
```

### Channel Information

```cpp
const std::string& getName() const;
```
Returns channel name (includes '#' prefix).

```cpp
const std::string& getTopic() const;
void setTopic(const std::string &topic);
```
Get/set channel topic.

### Member Management

```cpp
const std::map<std::string, int>& getMembers() const;
```
Returns map of channel members (nickname → socket fd).

```cpp
void addMember(const std::string& nickname, int fd);
void removeMember(const std::string& nickname);
bool isMember(const std::string& nickname) const;
```
Member addition, removal, and membership checking.

### Operator Management

```cpp
const std::set<std::string>& getOperators() const;
void addOperator(const std::string& nickname);
void removeOperator(const std::string& nickname);
bool isOperator(const std::string& nickname) const;
```
Channel operator management and privilege checking.

### Channel Modes

```cpp
bool isInviteOnly() const;
void setInviteOnly(bool inviteOnly);
```
Invite-only mode (+i) management.

```cpp
bool isTopicProtected() const;
void setTopicProtected(bool topicProtected);
```
Topic protection mode (+t) - only operators can change topic.

```cpp
const std::string& getPassword() const;
void setPassword(const std::string &password);
bool checkKey(const std::string& key) const;
```
Channel password/key (+k) management.

```cpp
int getUserLimit() const;
void setUserLimit(int limit);
```
User limit (+l) management. Setting to 0 removes the limit.

### Invite System

```cpp
const std::set<std::string>& getWhiteList() const;
void addToWhiteList(const std::string& nickname);
void removeFromWhiteList(const std::string& nickname);
bool isInvited(const std::string& nickname) const;
```
Invitation whitelist for invite-only channels.

### Message Broadcasting

```cpp
void broadcastMsg(const Client &sender, const Message &message) const;
```
Sends a message to all channel members except the sender.

## Message Class

**File**: `include/Message.hpp`, `src/Message.cpp`

Handles IRC message parsing and formatting according to RFC specifications.

### Constructor

```cpp
Message(const std::string& rawMessage);
```
Parses raw IRC message string into structured format.

**IRC Message Format**: `[:prefix] COMMAND [param1] [param2] [:trailing]`

### Accessors

```cpp
const std::string& getPrefix() const;
const std::string& getCommand() const;
const std::vector<std::string>& getParams() const;
const std::string& getTrailing() const;
```

### Utility Methods

```cpp
std::string toString() const;
```
Serializes message back to IRC protocol format.

```cpp
bool isValid() const;
```
Returns `true` if message was parsed successfully.

## Command Classes

**Base Class**: `include/Command.hpp`, `src/Command.cpp`

All IRC command implementations inherit from the base Command class.

### Base Command Interface

```cpp
class Command {
protected:
    Message inMessage_;
    
public:
    Command(const Message& msg);
    virtual ~Command();
    virtual void execute(Server& server, Client& sender) = 0;
    static Command* fromMessage(const Message& message);
};
```

### Command Factory

```cpp
static Command* fromMessage(const Message& message);
```
Creates appropriate command object based on message type. Returns `NULL` for unknown commands.

### Individual Commands

Each command class follows the same pattern:

```cpp
class ExampleCommand : public Command {
public:
    ExampleCommand(const Message& msg);
    static Command* fromMessage(const Message& message);
    virtual void execute(Server& server, Client& sender);
};
```

**Available Commands**:
- `NickCommand` - Change nickname
- `UserCommand` - Set user info  
- `PassCommand` - Server authentication
- `JoinCommand` - Join channels
- `PrivmsgCommand` - Send messages
- `QuitCommand` - Disconnect
- `KickCommand` - Remove users from channels
- `InviteCommand` - Invite users to channels
- `TopicCommand` - Manage channel topics
- `ModeCommand` - Channel and user mode management
- `WhoCommand` - List channel members

## Utility Classes

### CaseMappedString

**File**: `include/CaseMappedString.hpp`, `src/CaseMappedString.cpp`

Handles IRC case mapping rules for nickname comparison.

```cpp
class CaseMappedString {
public:
    CaseMappedString(const std::string& str);
    bool operator==(const CaseMappedString& other) const;
    bool operator<(const CaseMappedString& other) const;
    const std::string& getString() const;
};
```

**IRC Case Rules**: `A-Z` maps to `a-z`, `[]\^` maps to `{}|~`

### Debug Utilities

**File**: `include/Debug.hpp`

Color constants and debug macros:

```cpp
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

#ifdef DEBUG
#define debug(msg) std::cerr << YEL << __FILE__ << ":" << __LINE__ << " " << msg << RESET << std::endl;
#else
#define debug(msg) (void(0))
#endif
```

## Error Codes

**File**: `include/MessageType.hpp`

IRC numeric reply codes for error handling:

### Common Success Codes

- `RPL_WELCOME = 1` - Welcome message
- `RPL_YOURHOST = 2` - Host information
- `RPL_CREATED = 3` - Server creation time
- `RPL_MYINFO = 4` - Server information

### Common Error Codes

- `ERR_NOSUCHNICK = 401` - Nickname not found
- `ERR_NOSUCHCHANNEL = 403` - Channel not found
- `ERR_CANNOTSENDTOCHAN = 404` - Cannot send to channel
- `ERR_NORECIPIENT = 411` - No recipient specified
- `ERR_NOTEXTTOSEND = 412` - No text to send
- `ERR_UNKNOWNCOMMAND = 421` - Unknown command
- `ERR_NONICKNAMEGIVEN = 431` - No nickname given
- `ERR_ERRONEUSNICKNAME = 432` - Invalid nickname
- `ERR_NICKNAMEINUSE = 433` - Nickname already in use
- `ERR_NOTREGISTERED = 451` - Not registered with server
- `ERR_NEEDMOREPARAMS = 461` - Not enough parameters
- `ERR_ALREADYREGISTERED = 462` - Already registered

### Error Message Construction

```cpp
struct IrcErrorInfo {
    std::string code;
    std::string message;
    IrcErrorInfo(const std::string& c, const std::string& m);
};
```

Use the error codes with `Client::sendErrorMessage()` or `Server::broadcastErrorMessage()` for proper IRC protocol compliance.