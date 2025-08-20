# Architecture Diagrams

This document provides visual representations of the ft_irc server architecture.

## High-Level Architecture

```
                    ┌─────────────────────────────────────┐
                    │            IRC Clients              │
                    │  (irssi, HexChat, telnet, etc.)     │
                    └─────────────┬───────────────────────┘
                                  │ TCP/IP (port 6667)
                                  │
                    ┌─────────────▼───────────────────────┐
                    │          ft_irc Server             │
                    │                                     │
                    │  ┌─────────────┐ ┌────────────────┐ │
                    │  │   Server    │ │   Poll Event   │ │
                    │  │   Class     │ │   Loop         │ │
                    │  └─────────────┘ └────────────────┘ │
                    │                                     │
                    │  ┌─────────────┐ ┌────────────────┐ │
                    │  │   Client    │ │   Message      │ │
                    │  │ Management  │ │   Parsing      │ │
                    │  └─────────────┘ └────────────────┘ │
                    │                                     │
                    │  ┌─────────────┐ ┌────────────────┐ │
                    │  │  Channel    │ │   Command      │ │
                    │  │ Management  │ │   Execution    │ │
                    │  └─────────────┘ └────────────────┘ │
                    └─────────────────────────────────────┘
```

## Component Relationships

```
                    Server
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
    Clients        Channels       Messages
        │              │              │
        │              │              ▼
        │              │          Commands
        │              │              │
        │              └──────────────┤
        │                             │
        └─────────────────────────────┘
```

## Message Flow

```
1. Client Input
       │
       ▼
2. Raw Socket Data → [Buffer Management]
       │
       ▼  
3. Message Parsing → [Message Object]
       │
       ▼
4. Command Factory → [Command Object]
       │
       ▼
5. Command Execute → [Server State Update]
       │
       ▼
6. Response Generation → [Output Messages]
       │
       ▼
7. Message Distribution → [Target Clients]
```

## Class Hierarchy

```
                        Command
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    NickCommand    JoinCommand      PrivmsgCommand
    UserCommand    QuitCommand      KickCommand
    PassCommand    ModeCommand      InviteCommand
                   TopicCommand     WhoCommand
```

## Data Flow Architecture

```
                    ┌─────────────┐
                    │   Client    │
                    │ Connection  │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   Message   │
                    │   Buffer    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │    IRC      │
                    │   Parser    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  Command    │
                    │  Factory    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  Command    │
                    │  Execute    │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
        ┌─────────┐  ┌─────────┐  ┌─────────┐
        │ Client  │  │Channel  │  │ Server  │
        │ Update  │  │ Update  │  │ Update  │
        └─────────┘  └─────────┘  └─────────┘
              │            │            │
              └────────────┼────────────┘
                           │
                    ┌──────▼──────┐
                    │  Response   │
                    │ Generation  │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  Message    │
                    │Distribution │
                    └─────────────┘
```

## Event Loop Architecture

```
                    Main Event Loop
                          │
                    ┌─────▼─────┐
                    │   poll()  │
                    │   System  │
                    │    Call   │
                    └─────┬─────┘
                          │
              ┌───────────┼───────────┐
              │           │           │
              ▼           ▼           ▼
        New Connection  Data Ready  Error/Hangup
              │           │           │
              ▼           ▼           ▼
        Accept Socket  Read Message  Close Socket
              │           │           │
              ▼           ▼           ▼
         Add to Poll   Process Cmd   Remove Client
              │           │           │
              └───────────┼───────────┘
                          │
                    Continue Loop
```

## Memory Management

```
                    Server Object
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
        ▼                 ▼                 ▼
   std::vector<Client>    │       std::map<string,Channel>
        │                 │                 │
        │                 │                 │
        ▼                 ▼                 ▼
   Client Objects    Message Objects   Channel Objects
        │                 │                 │
        │                 │                 │
        ▼                 ▼                 ▼
   String Members    Dynamic Commands   Member Lists
                          │
                          ▼
                    (Auto-deleted after 
                     command execution)
```

## Thread Safety Model

```
Single-Threaded Architecture
              │
              ▼
     Non-blocking I/O (poll)
              │
              ▼
     Sequential Processing
              │
      ┌───────┼───────┐
      │       │       │
      ▼       ▼       ▼
   Accept   Read    Write
      │       │       │
      └───────┼───────┘
              │
              ▼
    No Synchronization
        Required
```

## File Structure Mapping

```
include/                    src/
├── Server.hpp     ────────►├── Server.cpp
├── Client.hpp     ────────►├── Client.cpp  
├── Channel.hpp    ────────►├── Channel.cpp
├── Message.hpp    ────────►├── Message.cpp
├── Command.hpp    ────────►├── Command.cpp
├── *Command.hpp   ────────►├── commands/
│                           │   ├── *Command.cpp
│                           │   └── ...
├── Debug.hpp      ────────►├── (No .cpp - header only)
└── ...                     ├── main.cpp
                            └── ...
```

## Protocol State Machine

```
Client Connection States:

    ┌─────────────┐
    │ CONNECTED   │
    └──────┬──────┘
           │ PASS command
           ▼
    ┌─────────────┐
    │   PASS_OK   │
    └──────┬──────┘
           │ NICK command  
           ▼
    ┌─────────────┐
    │   NICK_SET  │
    └──────┬──────┘
           │ USER command
           ▼
    ┌─────────────┐
    │AUTHENTICATED│ ◄─── Can execute commands
    └─────────────┘
```

## Command Processing Pipeline

```
Raw IRC Message: "PRIVMSG #test :Hello world"
        │
        ▼
Parse into Message object:
├── prefix: (empty)
├── command: "PRIVMSG"  
├── params: ["#test"]
└── trailing: "Hello world"
        │
        ▼
Command Factory creates PrivmsgCommand object
        │
        ▼
PrivmsgCommand::execute():
├── Validate authentication
├── Check channel membership
├── Validate parameters
└── Broadcast to channel members
        │
        ▼
For each channel member:
└── Send formatted message
```

---

These diagrams provide a visual overview of the ft_irc server architecture. For detailed implementation information, refer to the [DEVELOPER.md](DEVELOPER.md) guide.