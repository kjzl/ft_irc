# Development Workflow Guide

This document provides a step-by-step workflow for contributing to the ft_irc project as a developer.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Environment](#development-environment)
- [Git Workflow](#git-workflow)
- [Coding Standards](#coding-standards)
- [Testing Your Changes](#testing-your-changes)
- [Code Review Process](#code-review-process)
- [Release Process](#release-process)

## Getting Started

### Initial Setup

1. **Fork and Clone**:
   ```bash
   # Fork the repository on GitHub first
   git clone https://github.com/YOUR_USERNAME/ft_irc.git
   cd ft_irc
   
   # Add upstream remote
   git remote add upstream https://github.com/kjzl/ft_irc.git
   ```

2. **Verify Build**:
   ```bash
   make clean
   make debug
   ./ircserv 6667 testpass
   ```

3. **Read Documentation**:
   - [DEVELOPER.md](DEVELOPER.md) - Architecture and development guide
   - [API.md](API.md) - Class and method reference
   - [BUILD.md](BUILD.md) - Build system details
   - [TESTING.md](TESTING.md) - Testing procedures

## Development Environment

### Recommended Tools

**Compiler**: GCC 4.8+ or Clang 3.4+
```bash
# Install on Ubuntu/Debian
sudo apt-get install build-essential

# Install on macOS
xcode-select --install
```

**Debugging Tools**:
```bash
# Install debugging utilities
sudo apt-get install gdb valgrind

# Optional: Static analysis
sudo apt-get install cppcheck clang-tools
```

**IRC Testing Clients**:
```bash
# Command line clients
sudo apt-get install irssi weechat

# Alternative: Use netcat for basic testing
nc localhost 6667
```

### IDE Configuration

**VS Code** (recommended settings in `.vscode/`):
```json
{
    "C_Cpp.standard": "c++98",
    "C_Cpp.compilerArgs": ["-std=c++98", "-Wall", "-Wextra"],
    "files.associations": {
        "*.hpp": "cpp"
    }
}
```

**CLion/QtCreator**: Configure for C++98 standard in project settings.

### Environment Variables

For development, set these in your shell:

```bash
# ~/.bashrc or ~/.zshrc
export CXXFLAGS="-std=c++98 -Wall -Wextra -g3"
export IRC_DEBUG=1
```

## Git Workflow

### Branch Strategy

```bash
# Create feature branch
git checkout -b feature/add-new-command

# Create bugfix branch  
git checkout -b bugfix/fix-memory-leak

# Create documentation branch
git checkout -b docs/update-api-reference
```

### Commit Guidelines

**Commit Message Format**:
```
type(scope): brief description

Detailed explanation of changes (if needed)

Fixes #issue_number
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Adding tests
- `build`: Build system changes

**Examples**:
```bash
git commit -m "feat(server): add channel user limit enforcement"
git commit -m "fix(client): resolve memory leak in message buffer"
git commit -m "docs(api): update Channel class documentation"
```

### Keeping Updated

```bash
# Daily workflow
git fetch upstream
git checkout main
git merge upstream/main
git push origin main

# Rebase feature branch
git checkout feature/my-feature
git rebase main
```

## Coding Standards

### C++98 Compliance

**Required Patterns**:
```cpp
// Traditional for loops (no range-based)
for (std::vector<Client>::iterator it = clients.begin(); 
     it != clients.end(); ++it) {
    // Use *it
}

// Explicit type declarations (no auto)
std::map<std::string, Channel>::iterator channelIt = channels.find(name);

// NULL instead of nullptr
Client* client = NULL;

// Traditional containers initialization
std::vector<std::string> params;
params.reserve(5);
params.push_back("param1");
```

**Forbidden Features** (C++11+):
- `auto` keyword
- Range-based for loops
- Lambda functions
- `nullptr`
- Smart pointers (`unique_ptr`, `shared_ptr`)
- Initializer lists
- Move semantics

### Code Style

**Naming Conventions**:
```cpp
class ServerManager;           // PascalCase classes
void processMessage();         // camelCase methods
std::string serverName_;       // camelCase + underscore for members
const int MAX_CLIENTS = 100;   // UPPER_SNAKE_CASE constants
#define DEBUG_MODE             // UPPER_SNAKE_CASE macros
```

**File Organization**:
```cpp
// Header file structure
#ifndef CLASSNAME_HPP
#define CLASSNAME_HPP

// System includes first
#include <iostream>
#include <string>

// Project includes
#include "BaseClass.hpp"

class ClassName {
public:
    // Constructors first
    ClassName();
    ClassName(const ClassName& other);
    virtual ~ClassName();
    
    // Assignment operators
    ClassName& operator=(const ClassName& other);
    
    // Public methods (getters before setters)
    const std::string& getName() const;
    void setName(const std::string& name);
    
private:
    // Private methods
    void helperMethod();
    
    // Member variables (with trailing underscore)
    std::string name_;
    int count_;
};

#endif // CLASSNAME_HPP
```

### Error Handling

**Always handle errors**:
```cpp
// Check system calls
if (socket_fd < 0) {
    debug("Socket creation failed: " << strerror(errno));
    return false;
}

// Validate input parameters
if (nickname.empty()) {
    client.sendErrorMessage(ERR_NONICKNAMEGIVEN, args);
    return;
}

// Check container access
std::map<std::string, Channel>::iterator it = channels.find(channelName);
if (it == channels.end()) {
    client.sendErrorMessage(ERR_NOSUCHCHANNEL, args);
    return;
}
```

### Memory Management

```cpp
// RAII pattern - constructor acquires, destructor releases
class Client {
private:
    int socket_;
public:
    Client(int fd) : socket_(fd) {}
    ~Client() { 
        if (socket_ >= 0) {
            close(socket_);
        }
    }
};

// Dynamic allocation only when necessary
Command* cmd = CommandFactory::createCommand(message);
// ... use cmd
delete cmd;  // Always clean up

// Prefer stack allocation
std::vector<std::string> params;  // Automatic cleanup
```

## Testing Your Changes

### Pre-Commit Testing

**1. Build Tests**:
```bash
# Clean build
make fclean && make

# Debug build  
make debug

# Check for warnings
make 2>&1 | grep -i warning
```

**2. Static Analysis**:
```bash
# Check for common issues
cppcheck --enable=all src/ include/

# Memory leak detection
valgrind --leak-check=full ./ircserv 6667 pass
```

**3. Basic Functionality**:
```bash
# Terminal 1: Start server
./ircserv 6667 testpass

# Terminal 2: Test client
nc localhost 6667
PASS testpass
NICK alice
USER alice 0 * :Alice Test
JOIN #test
PRIVMSG #test :Hello world
QUIT :Testing complete
```

### Comprehensive Testing

Follow the testing procedures in [TESTING.md](TESTING.md):

1. **Multi-client testing**
2. **Error condition testing**
3. **IRC client compatibility**
4. **Performance testing**
5. **Security testing**

### Test Documentation

Document your testing:

```bash
# Create test log
echo "Testing feature: Add channel user limits" > test_log.txt
echo "Date: $(date)" >> test_log.txt
echo "Build: make debug" >> test_log.txt
echo "" >> test_log.txt

# Run tests and log results
./test_script.sh >> test_log.txt 2>&1
```

## Code Review Process

### Self-Review Checklist

Before submitting a pull request:

**Functionality**:
- [ ] Feature works as intended
- [ ] Error conditions handled properly
- [ ] No regressions in existing functionality
- [ ] IRC protocol compliance maintained

**Code Quality**:
- [ ] C++98 compliance verified
- [ ] Follows project naming conventions
- [ ] No memory leaks (Valgrind clean)
- [ ] Appropriate debug logging added
- [ ] Comments added for complex logic

**Testing**:
- [ ] Manual testing completed
- [ ] Multiple IRC clients tested
- [ ] Edge cases covered
- [ ] Performance impact assessed

### Preparing Pull Request

1. **Update Documentation**:
   ```bash
   # Update relevant docs if needed
   vim docs/API.md        # If adding new public methods
   vim docs/DEVELOPER.md  # If changing architecture
   vim COMMANDS.md        # If adding new IRC commands
   ```

2. **Clean Commit History**:
   ```bash
   # Squash commits if needed
   git rebase -i HEAD~3
   
   # Ensure clean history
   git log --oneline
   ```

3. **Final Validation**:
   ```bash
   # Full clean build
   make fclean && make
   
   # Run comprehensive tests
   ./validate_changes.sh
   ```

### Pull Request Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix
- [ ] New feature  
- [ ] Documentation update
- [ ] Code refactoring

## Testing
- [ ] Manual testing completed
- [ ] IRC client compatibility verified
- [ ] Memory leaks checked (Valgrind)
- [ ] Performance impact assessed

## Checklist
- [ ] Code follows project style guidelines
- [ ] C++98 compliance verified
- [ ] Documentation updated if needed
- [ ] Commit messages follow guidelines
```

## Release Process

### Version Management

```bash
# Update version in relevant files
vim include/Server.hpp  # Update VERSION constant
vim README.md          # Update version references

# Tag release
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

### Release Testing

Before release:

1. **Full regression testing**
2. **Performance benchmarks**
3. **Documentation review**
4. **Cross-platform testing**

### Documentation Updates

```bash
# Update changelog
vim CHANGELOG.md

# Update installation instructions if needed
vim INSTALL.md

# Review all documentation for accuracy
vim docs/*.md
```

## Troubleshooting Development Issues

### Common Problems

**Build Failures**:
```bash
# Check C++98 compliance
make CXXFLAGS="-std=c++98 -Wall -Wextra -Werror -pedantic"

# Clean build environment
make fclean
rm -rf obj/
make
```

**Runtime Issues**:
```bash
# Debug with GDB
make debug
gdb ./ircserv
(gdb) set args 6667 testpass
(gdb) run

# Memory debugging
valgrind --tool=memcheck --leak-check=full ./ircserv 6667 pass
```

**Git Issues**:
```bash
# Sync with upstream
git fetch upstream
git checkout main
git reset --hard upstream/main

# Fix merge conflicts
git mergetool
```

### Getting Help

1. **Check existing documentation** in `docs/`
2. **Search existing issues** on GitHub
3. **Ask specific questions** in issues
4. **Contact maintainers** for complex problems

---

Following this workflow ensures consistent, high-quality contributions to the ft_irc project.