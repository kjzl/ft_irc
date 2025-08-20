# Developer Documentation

This document provides comprehensive developer documentation for the **ft_irc** project, an IRC server implementation in C++98.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Development Setup](#development-setup)
- [Code Organization](#code-organization)
- [Core Classes](#core-classes)
- [Message Flow](#message-flow)
- [Command System](#command-system)
- [Development Workflow](#development-workflow)
- [Debugging](#debugging)
- [Testing](#testing)
- [Code Style Guidelines](#code-style-guidelines)
- [Adding New Features](#adding-new-features)

## Architecture Overview

The ft_irc server follows a modular, event-driven architecture built around these core concepts:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Client      │◄───┤     Server      ├───►│    Channel      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
    ┌────▼────┐              ┌───▼───┐              ┌────▼────┐
    │ Message │              │Command│              │  Mode   │
    └─────────┘              └───────┘              └─────────┘
```

### Key Components

- **Server**: Central coordinator managing clients, channels, and message routing
- **Client**: Individual IRC client connection with authentication state
- **Channel**: Chat rooms with member management and permissions
- **Message**: IRC protocol message parsing and formatting
- **Command**: Modular command handlers for IRC operations

### Design Patterns

- **Command Pattern**: Each IRC command is a separate class inheriting from `Command`
- **Observer Pattern**: Server broadcasts messages to interested clients
- **State Pattern**: Client registration levels track authentication progress
- **Factory Pattern**: Command creation from incoming messages

## Development Setup

### Prerequisites

- C++ compiler with C++98 support (GCC 4.8+ or Clang 3.4+)
- GNU Make
- Unix-like environment (Linux/macOS recommended)

### Build Configuration

The project uses a sophisticated Makefile with:

```makefile
# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 $(OPTIM_FLAGS)
OPTIM_FLAGS := -O3 -march=native

# Debug build
make debug  # Adds -DDEBUG -g3, removes optimizations
```

### Development Build

For active development, use the debug build:

```bash
make debug
```

This enables:
- Debug symbols (`-g3`)
- Debug macro definitions (`-DDEBUG`)
- Removes aggressive optimizations
- Enables debug logging via `debug()` macro

### Environment Setup

1. **Clone and build**:
   ```bash
   git clone https://github.com/kjzl/ft_irc.git
   cd ft_irc
   make debug
   ```

2. **Development cycle**:
   ```bash
   # Make changes
   make re          # Clean rebuild
   ./ircserv 6667 pass   # Test locally
   ```

3. **Testing setup**:
   ```bash
   # Terminal 1: Start server
   ./ircserv 6667 testpass
   
   # Terminal 2: Connect client
   nc localhost 6667
   ```

## Code Organization

```
ft_irc/
├── include/           # Header files
│   ├── Server.hpp     # Main server class
│   ├── Client.hpp     # Client connection management
│   ├── Channel.hpp    # Channel management
│   ├── Message.hpp    # IRC message parsing
│   ├── Command.hpp    # Base command class
│   ├── *Command.hpp   # Individual command headers
│   └── Debug.hpp      # Debug utilities
├── src/               # Implementation files
│   ├── main.cpp       # Entry point
│   ├── Server.cpp     # Server implementation
│   ├── Client.cpp     # Client implementation
│   ├── Channel.cpp    # Channel implementation
│   ├── Message.cpp    # Message parsing
│   └── commands/      # Command implementations
│       ├── NickCommand.cpp
│       ├── JoinCommand.cpp
│       └── ...
└── docs/              # Documentation
    └── DEVELOPER.md   # This file
```

### File Naming Conventions

- **Headers**: `ClassName.hpp`
- **Implementation**: `ClassName.cpp`
- **Commands**: `CommandNameCommand.{hpp,cpp}`
- **Utilities**: Descriptive names like `Debug.hpp`, `ircUtils.hpp`

## Core Classes

### Server Class

**Location**: `include/Server.hpp`, `src/Server.cpp`

**Responsibilities**:
- Socket management and event loop
- Client connection handling
- Message routing and broadcasting
- Channel and client lifecycle management

**Key Methods**:
```cpp
class Server {
public:
    Server(int port, std::string password);
    void waitForRequests();                    // Main event loop
    void broadcastMsg(const Message &message);  // Message distribution
    std::vector<Client>& getClients();         // Client access
    std::map<std::string, Channel>& getChannels(); // Channel access
};
```

### Client Class

**Location**: `include/Client.hpp`, `src/Client.cpp`

**Responsibilities**:
- IRC client state management
- Authentication tracking
- Message sending/receiving
- User information storage

**Key Concepts**:
- **Registration Levels**: Tracks PASS, NICK, USER command completion
- **Case-Mapped Nicknames**: Uses `CaseMappedString` for IRC case rules
- **Message Buffering**: Handles partial message reception

**Key Methods**:
```cpp
class Client {
public:
    void incrementRegistrationLevel();         // Authentication progress
    bool isAuthenticated() const;              // Full registration check
    void sendMessage(Message toSend) const;    // Send message to client
    void sendErrorMessage(MessageType type, std::vector<std::string>& args);
};
```

### Channel Class

**Location**: `include/Channel.hpp`, `src/Channel.cpp`

**Responsibilities**:
- Member management
- Operator permissions
- Channel modes (invite-only, topic protection, etc.)
- Message broadcasting within channel

**Key Features**:
- **Member tracking**: `std::map<std::string, int>` (nickname -> socket fd)
- **Operator system**: `std::set<std::string>` for operator nicknames
- **Mode system**: Boolean flags for channel behaviors
- **Invite system**: Whitelist for invite-only channels

### Message Class

**Location**: `include/Message.hpp`, `src/Message.cpp`

**Responsibilities**:
- IRC message parsing per RFC specifications
- Parameter extraction and validation
- Message formatting for transmission

**Message Format**: `:prefix COMMAND param1 param2 :trailing parameter`

### Command System

**Base Class**: `include/Command.hpp`, `src/Command.cpp`

All IRC commands inherit from the base `Command` class:

```cpp
class Command {
protected:
    Message inMessage_;
public:
    Command(const Message& msg);
    virtual void execute(Server& server, Client& sender) = 0;
    static Command* fromMessage(const Message& message);  // Factory method
};
```

**Command Implementation Pattern**:
1. Parse and validate parameters
2. Check client authentication state
3. Perform command-specific logic
4. Send appropriate responses or errors

## Message Flow

### Incoming Message Processing

```
Client Input → Message Parsing → Command Factory → Command Execute → Response
     │              │                 │                │             │
     │              │                 │                │             ▼
     │              │                 │                │        Send to Client(s)
     │              │                 │                │
     │              │                 │                ▼
     │              │                 │          Server State Update
     │              │                 │
     │              │                 ▼
     │              │           Command Object Creation
     │              │
     │              ▼
     │        Message Object Creation
     │
     ▼
Raw Socket Data → Buffer Management
```

### Event Loop

The server uses `poll()` for efficient I/O multiplexing:

```cpp
void Server::waitForRequests() {
    while (running_) {
        int activity = poll(pollFds_.data(), pollFds_.size(), TIMEOUT);
        
        for (size_t i = 0; i < pollFds_.size(); ++i) {
            if (pollFds_[i].revents & POLLIN) {
                if (i == 0) {
                    acceptConnection();  // New client
                } else {
                    processMessage(i);   // Existing client data
                }
            }
        }
    }
}
```

## Development Workflow

### Adding a New IRC Command

1. **Create header file** in `include/`:
   ```cpp
   // include/NewCommand.hpp
   #ifndef NEWCOMMAND_HPP
   #define NEWCOMMAND_HPP
   
   #include "Command.hpp"
   
   class NewCommand : public Command {
   public:
       NewCommand(const Message& msg);
       static Command* fromMessage(const Message& message);
       virtual void execute(Server& server, Client& sender);
   };
   
   #endif
   ```

2. **Implement in** `src/commands/`:
   ```cpp
   // src/commands/NewCommand.cpp
   #include "../../include/NewCommand.hpp"
   #include "../../include/Debug.hpp"
   
   NewCommand::NewCommand(const Message& msg) : Command(msg) {}
   
   Command* NewCommand::fromMessage(const Message& message) {
       return new NewCommand(message);
   }
   
   void NewCommand::execute(Server& server, Client& sender) {
       // Implementation here
   }
   ```

3. **Register in message parser** (`src/MessageType.cpp`):
   ```cpp
   command_map["NEWCOMMAND"] = NEWCOMMAND;
   ```

4. **Add to Makefile** in `SRCS` and `HDRS` sections

5. **Test thoroughly** with various clients and edge cases

### Code Review Checklist

- [ ] Follows C++98 standard (no C++11+ features)
- [ ] Proper error handling for all edge cases
- [ ] Memory management (no leaks, proper cleanup)
- [ ] IRC protocol compliance
- [ ] Consistent with existing code style
- [ ] Debug logging where appropriate
- [ ] Documentation comments for public interfaces

## Debugging

### Debug System

The project includes a comprehensive debug system in `include/Debug.hpp`:

```cpp
#ifdef DEBUG
#define debug(msg) \
    std::cerr << YEL << __FILE__ << ":" << __LINE__ << " " << msg << RESET << std::endl;
#else
#define debug(msg) (void(0))
#endif
```

**Usage**:
```cpp
debug("Client " << sender.getNickname() << " joined channel " << channelName);
```

### Color Codes

Available color constants for output formatting:
- `RED`, `GREEN`, `YEL`, `BLUE`, `MAG`, `CYN`, `WHT`
- `BRED` (Bold Red), `BOLD`, `RESET`

### Common Debugging Scenarios

1. **Connection Issues**:
   ```cpp
   debug("New connection from fd " << clientFd);
   debug("Client authentication level: " << client.getRegistrationLevel());
   ```

2. **Message Parsing**:
   ```cpp
   debug("Raw message: '" << rawMessage << "'");
   debug("Parsed command: " << message.getCommand());
   ```

3. **Channel Operations**:
   ```cpp
   debug("Channel " << channel.getName() << " has " << channel.getMembers().size() << " members");
   ```

### Valgrind Integration

The project includes `.valgrindrc` for memory debugging:

```bash
valgrind --track-origins=yes --leak-check=full ./ircserv 6667 pass
```

## Testing

### Manual Testing

Since there's no automated test suite, follow these manual testing procedures:

#### Basic Functionality Test

```bash
# Terminal 1: Start server
./ircserv 6667 testpass

# Terminal 2: Test client 1
nc localhost 6667
PASS testpass
NICK alice
USER alice 0 * :Alice Test
JOIN #test
PRIVMSG #test :Hello world

# Terminal 3: Test client 2  
nc localhost 6667
PASS testpass
NICK bob
USER bob 0 * :Bob Test
JOIN #test
PRIVMSG #test :Hi Alice!
```

#### Channel Operations Test

```bash
# Test channel modes
MODE #test +i           # Set invite-only
INVITE carol #test      # Invite user
MODE #test +t           # Topic protection
TOPIC #test :New topic  # Change topic
MODE #test +l 5         # User limit
KICK #test alice :bye   # Kick user
```

#### Error Condition Testing

- Invalid passwords
- Duplicate nicknames
- Non-existent channels
- Insufficient permissions
- Malformed commands

### IRC Client Testing

Test with real IRC clients:
- **irssi**: `irssi -c localhost -p 6667`
- **HexChat**: Connect to localhost:6667
- **WeeChat**: `/connect localhost/6667`

## Code Style Guidelines

### C++98 Compliance

**Forbidden Features** (C++11+):
- `auto` keyword
- Range-based for loops
- Lambda functions
- `nullptr` (use `NULL`)
- Smart pointers (`unique_ptr`, `shared_ptr`)

**Required Patterns**:
```cpp
// Use traditional for loops
for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
    // Use *it to access element
}

// Use traditional constructors
std::vector<std::string> vec;
vec.reserve(5);  // Explicit memory management
```

### Naming Conventions

- **Classes**: `PascalCase` (e.g., `ServerClass`, `MessageType`)
- **Methods**: `camelCase` (e.g., `getMessage()`, `setNickname()`)
- **Variables**: `camelCase` with trailing underscore for members (e.g., `nickname_`, `serverSocket_`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `HOSTNAME`, `BACKLOG`)
- **Macros**: `UPPER_SNAKE_CASE` (e.g., `DEBUG`, `RED`)

### Code Organization

- **Header Guards**: Use `#ifndef CLASSNAME_HPP`
- **Includes**: System headers first, then project headers
- **Method Order**: Public before private, constructors first
- **Comments**: Doxygen-style for public APIs

### Example Class Structure

```cpp
#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include <string>
#include <vector>
#include "BaseClass.hpp"

class ExampleClass : public BaseClass {
public:
    // Constructors
    ExampleClass();
    ExampleClass(const ExampleClass& other);
    virtual ~ExampleClass();
    
    // Assignment
    ExampleClass& operator=(const ExampleClass& other);
    
    // Public methods
    void publicMethod();
    const std::string& getName() const;

private:
    // Private methods
    void privateHelper();
    
    // Member variables
    std::string name_;
    std::vector<int> data_;
};

#endif // EXAMPLE_HPP
```

## Adding New Features

### Feature Development Process

1. **Design Phase**:
   - Review IRC RFCs for standard compliance
   - Design class interfaces and data structures
   - Consider backwards compatibility

2. **Implementation Phase**:
   - Create feature branch
   - Implement core functionality
   - Add comprehensive error handling
   - Include debug logging

3. **Testing Phase**:
   - Manual testing with multiple clients
   - Edge case validation
   - Performance testing under load

4. **Documentation Phase**:
   - Update relevant documentation files
   - Add code comments
   - Update command reference if applicable

### Common Extension Points

1. **New IRC Commands**: Follow command pattern in `src/commands/`
2. **Channel Modes**: Extend `Channel` class mode system
3. **Authentication**: Modify `Client` registration system  
4. **Message Types**: Add entries to `MessageType` enum
5. **Error Responses**: Add to IRC error code mappings

### Performance Considerations

- **Memory**: Use object pools for frequent allocations
- **CPU**: Avoid string copies in hot paths
- **Network**: Batch messages when possible
- **Scalability**: Consider client limits and resource usage

### Security Considerations

- **Input Validation**: Always validate user input
- **Buffer Overflows**: Use safe string operations
- **DOS Protection**: Rate limiting and resource limits
- **Authentication**: Secure password handling

---

For questions or clarifications about the codebase, consult the existing documentation or contact the project maintainers.