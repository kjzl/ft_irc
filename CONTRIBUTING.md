# Contributing to ft_irc

Thank you for your interest in contributing to **ft_irc**!  
This project was developed as a group project for the 42 curriculum by [haroldrf](https://github.com/haroldrf), [Moat423](https://github.com/Moat423), and [kjzl](https://github.com/kjzl).

## Quick Start for Contributors

For detailed development workflow and guidelines, see **[docs/WORKFLOW.md](./docs/WORKFLOW.md)**.

For technical details about the codebase, see **[docs/DEVELOPER.md](./docs/DEVELOPER.md)**.

## How to Contribute

- **Bug Reports & Issues:**
  If you discover a bug or have a suggestion, please open an issue on GitHub with as much detail as possible.

- **Pull Requests:**
  Pull requests are welcome for improvements, documentation, or bug fixes. Please ensure your code follows the C++98 standard and passes the testing procedures outlined in [docs/TESTING.md](./docs/TESTING.md).

- **Development Setup:**
  ```bash
  git clone https://github.com/kjzl/ft_irc.git
  cd ft_irc
  make debug
  ./ircserv 6667 testpass
  ```

- **Style Guide:**
  - Follow the C++98 standard strictly (no modern C++ features)
  - Use the formatting and naming conventions described in [docs/WORKFLOW.md](./docs/WORKFLOW.md)
  - Write clear commit messages following the project guidelines
  - Document your code where appropriate
  - Include comprehensive testing for new features

- **Testing Requirements:**
  All contributions must be thoroughly tested using the procedures in [docs/TESTING.md](./docs/TESTING.md), including:
  - Manual testing with multiple IRC clients
  - Error condition validation  
  - Memory leak detection with Valgrind
  - Protocol compliance verification

- **Questions:**
  For questions or clarifications, you can open an issue or contact one of the project authors directly.

## Developer Resources

- **[docs/README.md](./docs/README.md)** - Complete documentation index
- **[docs/WORKFLOW.md](./docs/WORKFLOW.md)** - Detailed development workflow
- **[docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)** - System architecture diagrams
- **[docs/API.md](./docs/API.md)** - Complete API reference
- **[docs/BUILD.md](./docs/BUILD.md)** - Build system documentation

Thank you for helping to improve ft_irc!
