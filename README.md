# ft_irc

A modern IRC (Internet Relay Chat) server implementation in C++  
_Group project by [haroldrf](https://github.com/haroldrf), [Moat423](https://github.com/Moat423), and [kjzl](https://github.com/kjzl)  
for the 42 Network Curriculum_

---

## Table of Contents

- [About the Project](#about-the-project)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Supported IRC Commands](#supported-irc-commands)
- [Documentation](#documentation)
- [Authors](#authors)
- [License](#license)
- [References](#references)

---

## About the Project

**ft_irc** is a comprehensive IRC server designed as a group project for the 42 Network.  
The project demonstrates proficiency in C++, socket programming, non-blocking I/O, protocol parsing, and concurrent network systems.  
It follows the IRC RFC (primarily RFC 1459 and modern extensions) and is suitable for both educational and demonstration purposes.

---

## Features

- Implements core IRC protocol and commands (RFC 1459)
- Channel creation and management (public channels, operator privileges, user limits)
- Multiple simultaneous client connections (using non-blocking sockets)
- Robust error handling and input validation
- Modular and extensible codebase for advanced features
- Clean code, comprehensive in-code documentation, and extensive external documentation

---

## Installation

See [INSTALL.md](./INSTALL.md) for detailed installation instructions.

**Quick Start:**
```sh
make
./ircserv <port> <password>
```
The standard IRC port is `6667`, so an example command is:
```sh
./ircserv 6667 mysecretpass
```

---

## Usage

See [USAGE.md](./USAGE.md) for detailed usage, client connection instructions, and advanced settings.

**Basic IRC client connection example:**
```sh
nc localhost 6667
NICK yournickname
USER yourident 0 * :Your Real Name
JOIN #mychannel
PRIVMSG #mychannel :Hello, world!
```

---

## Supported IRC Commands

Our server implements standard IRC commands:

- **PASS**: Authenticate with the server
- **NICK**: Set your nickname
- **USER**: Set your username and real name
- **JOIN**: Join a channel (channels start with `#`)
- **PRIVMSG**: Send a private message to a user or channel
- **NOTICE**: Send a notice to a user (alias of PRIVMSG)
- **MODE**: Set channel or user modes (see below)
- **KICK**: Channel operator command to eject a client
- **INVITE**: Operator command to invite a user to a channel
- **TOPIC**: Change or view the channel topic
- **WHO**: See who is in a channel

**Channel Modes:**
- `i`: Invite-only channel
- `t`: Only operators can change the topic
- `k`: Channel key (password)
- `o`: Give/take channel operator privilege
- `l`: Set/remove user limit (e.g., `+l 30` to limit to 30 users; `-l` removes the limit)

> **Note:** Setting a user limit of 0 (`+l 0`) is not valid and will be rejected, as per standard IRC protocol.

See [COMMANDS.md](./COMMANDS.md) for in-depth protocol and implementation details.

---

## Documentation

Extensive documentation is available in the [docs/](./docs/) directory, including:
- Architecture and code structure
- Protocol details and command reference
- Developer guides and diagrams
- References to [INSTALL.md](./INSTALL.md) and [USAGE.md](./USAGE.md)

---

## Authors

- [haroldrf](https://github.com/haroldrf)
- [Moat423](https://github.com/Moat423)
- [kjzl](https://github.com/kjzl)

---

## License

Distributed for educational use under the 42 school guidelines.  
Contact the authors for reuse permissions.

---

## References

- [IRCv3 Specifications](https://ircv3.net/irc/)
- [Modern IRC Docs](https://modern.ircdocs.horse/)
- [Non-blocking I/O in C](https://www.geeksforgeeks.org/c/non-blocking-io-with-pipes-in-c/)
- [Socket Programming in C++](https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/)
- [42 ft_irc Guide](https://reactive.so/post/42-a-comprehensive-guide-to-ft_irc/)
- [Small IRC Server for 42 Network](https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9)

---

_This project is part of the 42 Network curriculum and demonstrates advanced C++ and networking capabilities._
