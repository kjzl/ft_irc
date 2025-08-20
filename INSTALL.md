# Installation Guide

This document explains how to build and install **ft_irc** on your system.

---

## Prerequisites

- **Operating System:** Linux or macOS (recommended). Windows users may need WSL or a Unix-like environment.
- **Compiler:** `c++` supporting C++98 standard (e.g., GCC, Clang)
- **Make:** GNU Make
- **Network Access:** Required for IRC server operation

---

## Building the Project

1. **Clone the repository:**
   ```sh
   git clone https://github.com/kjzl/ft_irc.git
   cd ft_irc
   ```

2. **Build with Make:**
   ```sh
   make
   ```

   If the build is successful, you will see a message:
   ```
   Build successful!
   ```

   The built executable will be named `ircserv`.

---

## Running the Server

The syntax for starting the server is:
```sh
./ircserv <port> <password>
```
- `<port>`: The TCP port to listen on (use `6667` for standard IRC)
- `<password>`: The server password clients must use

**Example:**
```sh
./ircserv 6667 mysecretpass
```

---

## Cleaning and Rebuilding

- **Clean object files:**
  ```sh
  make clean
  ```
- **Clean all (including executable):**
  ```sh
  make fclean
  ```
- **Rebuild from scratch:**
  ```sh
  make re
  ```

---

## Debug Build

To build with debug information (adds symbols and disables optimizations):

```sh
make debug
```

---

## Troubleshooting

- If you encounter compiler errors, ensure your C++ compiler supports C++98 and all dependencies are installed.
- For permissions issues, ensure you have rights to the project directory and the chosen network port (ports <1024 may require root).
- For port conflicts, ensure no other service is using your chosen port.

---

## Additional Notes

- No external dependencies are required beyond standard C++ and Make.
- For further usage instructions, see [USAGE.md](./USAGE.md).
- For protocol and command details, see [COMMANDS.md](./COMMANDS.md).
- For advanced configuration and documentation, see [docs/](./docs/).

---

If you have special installation requirements or wish to run on other platforms, please open an issue or contact the authors.
