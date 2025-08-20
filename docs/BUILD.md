# Build System Documentation

This document explains the build system and development environment setup for the ft_irc project.

## Table of Contents

- [Makefile Overview](#makefile-overview)
- [Build Targets](#build-targets)
- [Compiler Configuration](#compiler-configuration)
- [Development Builds](#development-builds)
- [Directory Structure](#directory-structure)
- [Customizing the Build](#customizing-the-build)
- [Troubleshooting](#troubleshooting)

## Makefile Overview

The ft_irc project uses a sophisticated GNU Makefile with the following features:

- **C++98 Standard Compliance**: Enforced through compiler flags
- **Optimized Production Builds**: With `-O3` and architecture-specific optimizations
- **Debug Build Support**: Separate debug target with symbols and logging
- **Progress Indicators**: Visual build progress with colored output
- **Automatic Dependency Management**: Header file tracking
- **Clean Separation**: Object files in dedicated directory

### Key Makefile Variables

```makefile
NAME := ircserv                    # Executable name
CXX := c++                         # Compiler (c++ symlink to system default)
OPTIM_FLAGS := -O3 -march=native   # Optimization flags
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 $(OPTIM_FLAGS)
```

## Build Targets

### Primary Targets

#### `make` or `make all`
Builds the optimized production version of the IRC server.

```bash
make
# or
make all
```

**Features**:
- Maximum optimization (`-O3`)
- Architecture-specific optimizations (`-march=native`)
- All warnings as errors (`-Werror`)
- No debug symbols (smaller binary)

#### `make debug`
Builds a debug version with development-friendly features.

```bash
make debug
```

**Features**:
- Debug symbols (`-g3`)
- Debug macro definitions (`-DDEBUG`)
- Disables aggressive optimizations
- Enables `debug()` macro logging
- Larger binary but easier to debug

#### `make re`
Clean rebuild - removes all build artifacts and rebuilds from scratch.

```bash
make re
```
Equivalent to `make fclean && make`

### Maintenance Targets

#### `make clean`
Removes object files but keeps the executable.

```bash
make clean
```

Removes:
- `obj/` directory and all `.o` files
- Build flag tracking files

#### `make fclean`
Full clean - removes all build artifacts including the executable.

```bash
make fclean
```

Removes everything from `make clean` plus:
- `ircserv` executable

### Convenience Targets

#### `make run`
Builds and runs the server (useful for quick testing).

```bash
make run
```

Note: You'll need to manually specify port and password when the server starts.

## Compiler Configuration

### Base Flags

```makefile
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 $(OPTIM_FLAGS)
```

**Flag Breakdown**:
- `-Wall -Wextra`: Enable comprehensive warnings
- `-Werror`: Treat warnings as errors (ensures clean code)
- `-pedantic`: Strict ISO C++ compliance
- `-std=c++98`: Force C++98 standard (no modern C++ features)

### Optimization Flags

**Production** (`make`):
```makefile
OPTIM_FLAGS := -O3 -march=native
```
- `-O3`: Maximum optimization level
- `-march=native`: Optimize for current CPU architecture

**Debug** (`make debug`):
```makefile
CXXFLAGS += -DDEBUG -g3
```
- `-DDEBUG`: Enable debug macro compilation
- `-g3`: Maximum debug information
- No aggressive optimizations (easier debugging)

### Include Paths

```makefile
INCLUDES := -I$(HDRS_DIR)
```
Points to `include/` directory for header files.

## Development Builds

### Debug Build Usage

For active development, always use the debug build:

```bash
make debug
./ircserv 6667 testpass
```

**Debug Features Enabled**:

1. **Debug Logging**: Use the `debug()` macro anywhere in code:
   ```cpp
   debug("Client " << nickname << " joined channel " << channelName);
   ```

2. **Color Output**: Predefined color constants:
   ```cpp
   std::cout << GREEN << "Success!" << RESET << std::endl;
   ```

3. **File/Line Information**: Debug output shows source location:
   ```
   src/Server.cpp:123 New client connected fd=4
   ```

4. **Better Debugging**: GDB and Valgrind work better with debug symbols:
   ```bash
   gdb ./ircserv
   valgrind --leak-check=full ./ircserv 6667 pass
   ```

### Development Cycle

```bash
# 1. Make changes to source files
vim src/Server.cpp

# 2. Rebuild (incremental)
make debug

# 3. Test changes
./ircserv 6667 testpass

# 4. If major changes, clean rebuild
make re

# 5. Final testing with production build
make fclean
make
./ircserv 6667 testpass
```

## Directory Structure

### Source Organization

```
ft_irc/
├── src/              # Source files (.cpp)
│   ├── main.cpp      # Entry point
│   ├── *.cpp         # Core implementation files
│   └── commands/     # Command implementations
├── include/          # Header files (.hpp)
├── obj/              # Build artifacts (created by make)
│   ├── *.o           # Object files
│   └── commands/     # Command object files
└── ircserv           # Final executable
```

### Makefile File Lists

**Source Files** (automatically compiled):
```makefile
SRCS := $(addprefix $(SRCS_DIR)/,\
    main.cpp \
    Server.cpp \
    Client.cpp \
    Message.cpp \
    MessageType.cpp \
    CaseMappedString.cpp \
    Command.cpp \
    Channel.cpp \
    commands/NickCommand.cpp \
    commands/PassCommand.cpp \
    # ... more commands
)
```

**Header Files** (dependency tracking):
```makefile
HDRS := $(addprefix $(HDRS_DIR)/,\
    CaseMappedString.hpp \
    Channel.hpp \
    Client.hpp \
    # ... more headers
)
```

### Build Progress

The Makefile includes a visual progress indicator:

```
Compiling: [=================>      ] 67% src/commands/JoinCommand.cpp
```

Features:
- Progress bar showing completion percentage
- Current file being compiled
- Color-coded output (yellow during compilation, green on success)

## Customizing the Build

### Adding New Source Files

1. **Add to SRCS list** in Makefile:
   ```makefile
   SRCS := $(addprefix $(SRCS_DIR)/,\
       # existing files...
       commands/NewCommand.cpp \  # Add your new file
   )
   ```

2. **Add header to HDRS list**:
   ```makefile
   HDRS := $(addprefix $(HDRS_DIR)/,\
       # existing headers...
       NewCommand.hpp \           # Add your new header
   )
   ```

3. **Rebuild dependencies**:
   ```bash
   make re
   ```

### Custom Compiler

Override the default compiler:

```bash
make CXX=clang++
# or
make CXX=g++-9
```

### Custom Flags

Add extra compiler flags:

```bash
make CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -O2 -DEXTRA_FEATURE"
```

### Cross-Platform Builds

For different architectures:

```bash
# Generic build (no native optimizations)
make OPTIM_FLAGS="-O3"

# Specific architecture
make OPTIM_FLAGS="-O3 -march=x86-64"
```

## Troubleshooting

### Common Build Issues

#### 1. Compiler Not Found

**Error**: `make: c++: Command not found`

**Solutions**:
```bash
# Install GCC/G++
sudo apt-get install build-essential  # Ubuntu/Debian
sudo yum install gcc-c++              # CentOS/RHEL

# Or use specific compiler
make CXX=g++
make CXX=clang++
```

#### 2. C++98 Compliance Errors

**Error**: `error: 'auto' was not declared in this scope`

**Solution**: Remove C++11+ features:
```cpp
// Wrong (C++11+)
auto it = container.begin();

// Correct (C++98)
std::container<type>::iterator it = container.begin();
```

#### 3. Header Not Found

**Error**: `fatal error: SomeHeader.hpp: No such file or directory`

**Solutions**:
- Check file exists in `include/` directory
- Verify filename spelling and case
- Add to `HDRS` list in Makefile if new file

#### 4. Linker Errors

**Error**: `undefined reference to 'SomeClass::someMethod()'`

**Solutions**:
- Check implementation exists in corresponding `.cpp` file
- Verify function signature matches declaration
- Add source file to `SRCS` list in Makefile

#### 5. Permission Errors

**Error**: `Permission denied` when running server

**Solutions**:
```bash
# Use port > 1024 (doesn't require root)
./ircserv 6667 pass

# Or run as root for port < 1024 (not recommended for development)
sudo ./ircserv 80 pass
```

### Build Performance

#### Parallel Builds

Enable parallel compilation:
```bash
make -j$(nproc)  # Use all CPU cores
make -j4         # Use 4 cores
```

#### Incremental Builds

The Makefile supports incremental builds - only changed files are recompiled:

```bash
# First build: compiles everything
make debug

# Change one file
echo "// comment" >> src/Client.cpp

# Second build: only recompiles Client.cpp and relinks
make debug
```

### Debug Build Troubleshooting

#### Debug Output Not Appearing

1. **Verify debug build**:
   ```bash
   make debug
   ./ircserv 6667 pass
   ```

2. **Check debug macro usage**:
   ```cpp
   #include "Debug.hpp"
   debug("This should appear in debug builds");
   ```

3. **Redirect stderr** (debug output goes to stderr):
   ```bash
   ./ircserv 6667 pass 2>&1 | tee debug.log
   ```

### Memory Debugging

#### Valgrind Integration

The project includes `.valgrindrc` configuration:

```bash
# Run with memory checking
valgrind ./ircserv 6667 pass

# Detailed leak checking
valgrind --leak-check=full --track-origins=yes ./ircserv 6667 pass
```

#### AddressSanitizer

For faster memory debugging:

```bash
make CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address"
./ircserv 6667 pass
```

## Advanced Build Features

### Dependency Tracking

The Makefile automatically tracks header dependencies:

- Changing a header file rebuilds all source files that include it
- Uses make's built-in dependency generation
- No manual dependency management required

### Build Flags Persistence

The build system tracks compiler flags:

- Changing CXXFLAGS triggers a rebuild
- Ensures consistency between builds
- Stored in `$(OBJ_DIR)/.flags`

### Colored Output

Build output uses color coding:

- **Yellow**: Currently compiling
- **Green**: Successful build
- **Red**: Build errors
- **Bold**: Important messages

Disable colors for CI/scripting:
```bash
make NO_COLOR=1
```