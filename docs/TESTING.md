# Testing and Quality Assurance Guide

This document provides comprehensive testing strategies and quality assurance practices for the ft_irc project.

## Table of Contents

- [Testing Philosophy](#testing-philosophy)
- [Manual Testing](#manual-testing)
- [IRC Client Testing](#irc-client-testing)
- [Protocol Compliance Testing](#protocol-compliance-testing)
- [Performance Testing](#performance-testing)
- [Security Testing](#security-testing)
- [Debugging Techniques](#debugging-techniques)
- [Quality Assurance Checklist](#quality-assurance-checklist)
- [Common Issues and Solutions](#common-issues-and-solutions)

## Testing Philosophy

The ft_irc project follows a comprehensive manual testing approach due to the interactive nature of IRC protocols. Testing focuses on:

1. **Protocol Compliance**: Ensuring adherence to IRC RFCs
2. **Multi-Client Scenarios**: Testing concurrent connections and interactions
3. **Edge Case Handling**: Validating error conditions and boundary cases
4. **Real-World Usage**: Testing with actual IRC clients
5. **Performance Under Load**: Stress testing with multiple connections

## Manual Testing

### Basic Connectivity Test

#### Test Setup

```bash
# Terminal 1: Start server with debug output
make debug
./ircserv 6667 testpass

# Terminal 2: Test client
nc localhost 6667
```

#### Core Authentication Flow

```irc
# Test basic registration sequence
PASS testpass
NICK alice
USER alice 0 * :Alice Test User

# Expected: Welcome messages (001-004 numeric codes)
# Should see registration complete
```

#### Validation Points

- [ ] Server accepts connection
- [ ] PASS command validates password
- [ ] NICK command sets nickname uniquely
- [ ] USER command completes registration
- [ ] Welcome sequence sent correctly
- [ ] Client marked as authenticated

### Channel Operations Test

#### Basic Channel Functionality

```irc
# Create and join channel
JOIN #test

# Send messages
PRIVMSG #test :Hello everyone!
NOTICE #test :This is a notice

# Check channel status
WHO #test
```

#### Channel Modes Testing

```irc
# Test invite-only mode
MODE #test +i
JOIN #test        # Should fail for unauthorized users

# Test topic protection
MODE #test +t
TOPIC #test :New topic    # Should work for operators only

# Test user limit
MODE #test +l 5
# Test with 6+ users attempting to join

# Test channel key
MODE #test +k secretkey
JOIN #test wrongkey      # Should fail
JOIN #test secretkey     # Should succeed
```

#### Operator Commands

```irc
# Test operator privileges
MODE #test +o bob        # Give operator status
KICK #test alice :Reason # Kick user
INVITE carol #test       # Invite user to invite-only channel
```

### Multi-Client Testing

#### Two-Client Scenario

```bash
# Terminal 1: Server
./ircserv 6667 testpass

# Terminal 2: Client 1 (Alice)
nc localhost 6667
PASS testpass
NICK alice
USER alice 0 * :Alice Test
JOIN #chat

# Terminal 3: Client 2 (Bob) 
nc localhost 6667
PASS testpass
NICK bob
USER bob 0 * :Bob Test
JOIN #chat

# Test communication
# Alice: PRIVMSG #chat :Hi Bob!
# Bob: PRIVMSG #chat :Hello Alice!
```

#### Validation Points

- [ ] Both clients receive welcome messages
- [ ] Channel membership shows both users
- [ ] Messages broadcast to all channel members
- [ ] Private messages work between users
- [ ] WHO command shows all channel members

### Error Condition Testing

#### Authentication Errors

```irc
# Test wrong password
PASS wrongpass
NICK test
USER test 0 * :Test
# Expected: ERR_PASSWDMISMATCH

# Test duplicate nicknames
# Client 1: NICK alice
# Client 2: NICK alice
# Expected: ERR_NICKNAMEINUSE for Client 2

# Test commands before authentication
PRIVMSG #test :hello
# Expected: ERR_NOTREGISTERED
```

#### Command Parameter Errors

```irc
# Test missing parameters
NICK                    # ERR_NONICKNAMEGIVEN
PRIVMSG                 # ERR_NORECIPIENT  
PRIVMSG alice           # ERR_NOTEXTTOSEND
JOIN                    # ERR_NEEDMOREPARAMS

# Test invalid targets
PRIVMSG nonexistent :hi # ERR_NOSUCHNICK
JOIN #nonexistent       # Should create channel
PRIVMSG #restricted :hi # ERR_CANNOTSENDTOCHAN (if not member)
```

#### Channel Access Errors

```irc
# Test invite-only channel access
MODE #private +i
# Non-operator trying to join: ERR_INVITEONLYCHAN

# Test channel key errors  
MODE #secure +k password
JOIN #secure wrongkey    # ERR_BADCHANNELKEY

# Test user limit
MODE #limited +l 2
# Third user joining: ERR_CHANNELISFULL
```

## IRC Client Testing

### Recommended IRC Clients

#### Command Line Clients

**irssi**:
```bash
irssi -c localhost -p 6667
/password testpass
/nick alice
/join #test
```

**WeeChat**:
```bash
weechat
/connect localhost/6667
/password testpass
/nick alice
/join #test
```

#### GUI Clients

**HexChat** (Linux/Windows):
1. Add server: localhost:6667
2. Set server password: testpass
3. Connect and test functionality

**IRCCloud** (Web-based):
1. Add custom server
2. Test web client compatibility

### Client Compatibility Testing

#### Test Matrix

| Feature | irssi | WeeChat | HexChat | nc/telnet |
|---------|-------|---------|---------|-----------|
| Basic connection | ✓ | ✓ | ✓ | ✓ |
| Channel joining | ✓ | ✓ | ✓ | ✓ |
| Private messages | ✓ | ✓ | ✓ | ✓ |
| Channel modes | ✓ | ✓ | ✓ | Manual |
| WHO command | ✓ | ✓ | ✓ | ✓ |
| Operator commands | ✓ | ✓ | ✓ | Manual |

#### Compatibility Validation

- [ ] All major IRC clients can connect
- [ ] Message formatting displays correctly
- [ ] Channel operations work as expected
- [ ] Error messages are properly handled
- [ ] Disconnection is handled gracefully

## Protocol Compliance Testing

### IRC RFC Compliance

#### Message Format Validation

Test IRC message parsing with various formats:

```irc
# Standard format
PRIVMSG #test :Hello world

# With prefix (server-generated)
:alice!alice@host PRIVMSG #test :Hello

# Multiple parameters
MODE #test +o alice

# Empty trailing parameter
PRIVMSG #test :

# Long messages (512 character limit)
PRIVMSG #test :[479 character message...]
```

#### Numeric Replies

Verify correct numeric codes are sent:

```irc
# Welcome sequence
001 - RPL_WELCOME
002 - RPL_YOURHOST  
003 - RPL_CREATED
004 - RPL_MYINFO

# Error codes
401 - ERR_NOSUCHNICK
403 - ERR_NOSUCHCHANNEL
404 - ERR_CANNOTSENDTOCHAN
433 - ERR_NICKNAMEINUSE
461 - ERR_NEEDMOREPARAMS
```

#### Case Sensitivity

Test IRC case mapping rules:

```irc
# Nicknames should be case-insensitive
NICK Alice
NICK alice    # Should be treated as duplicate

# Channel names
JOIN #Test
JOIN #test    # Should be same channel

# Special characters: []\ should map to {}|
NICK test[
NICK test{    # Should be duplicate
```

### Standards Compliance Checklist

- [ ] Message length limits (512 characters)
- [ ] Proper CRLF line endings
- [ ] Case-insensitive nickname/channel matching
- [ ] Correct numeric reply codes
- [ ] Proper error message formats
- [ ] Channel name validation (# prefix)
- [ ] Nickname character restrictions

## Performance Testing

### Load Testing

#### Multiple Client Connections

```bash
# Script to create multiple connections
for i in {1..50}; do
    {
        echo "PASS testpass"
        echo "NICK user$i"
        echo "USER user$i 0 * :User $i"
        echo "JOIN #loadtest"
        sleep 1
        echo "PRIVMSG #loadtest :Message from user $i"
        sleep 5
        echo "QUIT :Load test complete"
    } | nc localhost 6667 &
done
wait
```

#### Performance Metrics

Monitor during load testing:

```bash
# System resources
top -p $(pgrep ircserv)
htop -p $(pgrep ircserv)

# Memory usage
valgrind --tool=massif ./ircserv 6667 pass

# Network connections
netstat -an | grep :6667
ss -tn | grep :6667
```

#### Performance Benchmarks

Target performance metrics:

- **Concurrent Connections**: 100+ simultaneous clients
- **Message Throughput**: 1000+ messages/second
- **Memory Usage**: <10MB per 100 clients
- **CPU Usage**: <50% under normal load
- **Response Time**: <100ms for command processing

### Stress Testing Scenarios

#### Channel Message Flooding

```bash
# High-frequency message test
for i in {1..1000}; do
    echo "PRIVMSG #stress :Message $i" | nc localhost 6667
done
```

#### Rapid Connect/Disconnect

```bash
# Connection churn test
for i in {1..100}; do
    {
        echo "PASS testpass"
        echo "NICK temp$i"
        echo "USER temp$i 0 * :Temp $i"
        echo "QUIT :Stress test"
    } | nc localhost 6667 &
done
```

## Security Testing

### Input Validation Testing

#### Buffer Overflow Protection

```irc
# Test extremely long inputs
NICK [1000+ character string]
PRIVMSG #test :[10000+ character message]
USER [long] 0 * :[very long real name]
```

#### Special Character Handling

```irc
# Control characters
NICK test\x00\x01\x02
PRIVMSG #test :\x03\x04\x05

# Unicode/UTF-8
NICK tëst
PRIVMSG #test :héllo wörld 🌍

# IRC formatting codes
PRIVMSG #test :\x02bold\x02 \x03,5colored\x03
```

#### Command Injection

```irc
# Attempt command injection in parameters
NICK alice; DROP TABLE users;
PRIVMSG #test; JOIN #evil :injected text
```

### Authentication Security

#### Password Security

```irc
# Test password edge cases
PASS ""           # Empty password
PASS " "          # Whitespace password  
PASS [very long]  # Long password
PASS \x00\x01     # Control characters
```

#### Session Management

```irc
# Test duplicate sessions
# Client 1: Connect and authenticate as alice
# Client 2: Try to connect as alice (should fail)

# Test session timeout
# Connect, authenticate, then idle for extended period
```

### Denial of Service Protection

#### Resource Exhaustion

```bash
# Connection flooding
for i in {1..1000}; do
    nc localhost 6667 &
done

# Memory exhaustion through large channels
# Create channels with many members
# Send large messages repeatedly
```

## Debugging Techniques

### Debug Build Usage

```bash
# Compile with debug information
make debug

# Run with debug output
./ircserv 6667 testpass 2>&1 | tee debug.log

# Filter debug output
./ircserv 6667 testpass 2>&1 | grep -E "(ERROR|Client|Channel)"
```

### GDB Debugging

```bash
# Compile with debug symbols
make debug

# Run in GDB
gdb ./ircserv
(gdb) set args 6667 testpass
(gdb) run
(gdb) bt       # Backtrace on crash
(gdb) info locals
(gdb) p variable_name
```

### Valgrind Analysis

```bash
# Memory leak detection
valgrind --leak-check=full --track-origins=yes ./ircserv 6667 pass

# Memory usage profiling
valgrind --tool=massif ./ircserv 6667 pass
ms_print massif.out.* 

# Thread safety (if applicable)
valgrind --tool=helgrind ./ircserv 6667 pass
```

### Network Debugging

```bash
# Monitor network traffic
tcpdump -i lo -A port 6667

# Wireshark packet capture
wireshark -i lo -f "port 6667"

# Connection monitoring
lsof -i :6667
netstat -tlnp | grep 6667
```

## Quality Assurance Checklist

### Pre-Release Testing

#### Functionality Testing

- [ ] All IRC commands work correctly
- [ ] Error conditions are handled properly
- [ ] Multi-client scenarios function as expected
- [ ] Channel operations work correctly
- [ ] Authentication system is secure
- [ ] Message broadcasting works properly

#### Compatibility Testing

- [ ] Works with major IRC clients (irssi, HexChat, WeeChat)
- [ ] Cross-platform compatibility (Linux, macOS)
- [ ] Different compiler versions (GCC, Clang)
- [ ] Various system configurations

#### Performance Testing

- [ ] Handles expected load (50+ concurrent clients)
- [ ] Memory usage remains reasonable
- [ ] No memory leaks detected
- [ ] CPU usage acceptable under load
- [ ] Network efficiency maintained

#### Security Testing

- [ ] Input validation prevents crashes
- [ ] No buffer overflow vulnerabilities
- [ ] Authentication cannot be bypassed
- [ ] No privilege escalation possible
- [ ] DoS resistance adequate

### Code Quality Checks

#### Static Analysis

```bash
# Compiler warnings (already enabled with -Wall -Wextra)
make

# Additional static analysis tools
cppcheck --enable=all src/
clang-static-analyzer src/

# Code formatting consistency
# (Manual review - project doesn't use automated formatting)
```

#### Code Review Points

- [ ] C++98 compliance throughout
- [ ] Memory management (no leaks, proper cleanup)
- [ ] Error handling comprehensive
- [ ] Code follows project conventions
- [ ] Comments and documentation adequate
- [ ] No hardcoded values where constants should be used

## Common Issues and Solutions

### Build Issues

**Issue**: Compilation fails with C++11+ errors
**Solution**: Remove modern C++ features, use C++98 equivalents

**Issue**: Linker errors for missing symbols
**Solution**: Check Makefile includes all source files

### Runtime Issues

**Issue**: Server crashes on client disconnect
**Solution**: Check socket cleanup and client removal logic

**Issue**: Messages not delivered to channel members
**Solution**: Verify channel membership tracking and message broadcasting

**Issue**: Memory leaks detected
**Solution**: Review object lifecycle management, ensure proper cleanup

### Network Issues

**Issue**: Clients cannot connect
**Solution**: Check firewall, port availability, and server binding

**Issue**: Messages arrive out of order
**Solution**: Review message queuing and socket handling

**Issue**: Connection timeouts
**Solution**: Implement proper keep-alive and timeout handling

### Protocol Issues

**Issue**: IRC clients show formatting errors
**Solution**: Verify IRC message format compliance (CRLF endings, etc.)

**Issue**: Channel modes not working
**Solution**: Check mode parsing and state management

**Issue**: Operator commands fail
**Solution**: Verify permission checking and operator status tracking

---

This testing guide should be followed for any changes to the codebase to ensure reliability and compliance with IRC standards.