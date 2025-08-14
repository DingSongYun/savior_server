# Boost.Asio Learning Project

A comprehensive, self-contained learning project for mastering Boost.Asio networking library. This project is completely independent and has no dependencies on the parent mobileserver project.

## 🚀 Quick Start

### Prerequisites
- C++17 compatible compiler (Visual Studio 2019+, GCC 8+, Clang 7+)
- CMake 3.15 or later
- Boost libraries (optional, see installation guide)

### Build and Run

```bash
# Navigate to project directory
cd examples/boost_asio_learning

# Build the project (Windows)
cd build
.\build.bat

# Build the project (Linux)
cd build
./build.sh
```

## 📁 Project Structure

```
boost_asio_learning/
├── README.md                           # This file
├── CMakeLists.txt                      # Build configuration
├── docs/                               # Documentation
│   ├── learning_guide.md               # Step-by-step learning guide
│   └── boost_installation.md           # Boost installation instructions
├── basic_examples/                     # Progressive learning examples
│   ├── 01_basic_concepts.cpp           # io_context, async basics
│   ├── 02_timers.cpp                  # Asynchronous timers
│   ├── 03_networking_basics.cpp        # TCP/UDP networking
│   ├── 04_advanced_networking.cpp      # Advanced networking features
│   └── 05_strand_thread_safety.cpp     # Thread safety with strands
├── projects/                           # Practical projects
│   └── chat_server/                    # Multi-client chat server
│       ├── chat_server.cpp             # Server implementation
│       ├── chat_client.cpp             # Client implementation
│       └── README.md                   # Project documentation
├── build/                              # Build scripts
│   ├── build.bat                       # Windows build script
│   └── build.sh                        # Linux build script
└── tools/                              # Utility scripts
    ├── setup_env.bat                   # Environment setup
    └── test_runner.py                  # Test automation
```

## 📚 Learning Path

### 1. Setup Environment
- Read `docs/boost_installation.md` for Boost installation
- Run `tools/setup_env.bat` (Windows) for environment setup

### 2. Study Documentation
- Start with `docs/learning_guide.md` for comprehensive guidance
- Understand the project structure and goals

### 3. Work Through Examples
Progress through the basic examples in order:
1. **01_basic_concepts.cpp** - Understanding io_context and async operations
2. **02_timers.cpp** - Asynchronous timers and callbacks
3. **03_networking_basics.cpp** - TCP/UDP socket programming
4. **04_advanced_networking.cpp** - Advanced networking patterns
5. **05_strand_thread_safety.cpp** - Thread safety and synchronization

### 4. Build Practical Projects
- Implement the chat server in `projects/chat_server/`
- Extend with your own networking projects

## 🔧 Build System Features

### Intelligent Configuration
- **Boost Detection**: Automatically detects Boost installation
- **Fallback Mode**: Builds simple hello program if Boost not found
- **Cross-Platform**: Supports Windows (Visual Studio) and Linux (GCC/Clang)

### Build Options
```bash
# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info (default)
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

## 🎯 Project Goals

### Learning Objectives
- Master asynchronous programming concepts
- Understand Boost.Asio's design patterns
- Build real-world networking applications
- Learn thread safety and performance optimization

### Practical Skills
- TCP/UDP server and client development
- Asynchronous I/O operations
- Error handling in network programming
- Multi-threaded networking applications

## 🛠️ Development Features

### Code Organization
- **Progressive Examples**: From basic to advanced concepts
- **Well-Commented Code**: Extensive explanations and best practices
- **Modular Design**: Each example focuses on specific concepts
- **Real-World Projects**: Practical applications you can extend

### Build System
- **Automated Building**: One-command build process
- **Dependency Management**: Intelligent Boost detection
- **Cross-Platform**: Windows and Linux support
- **IDE Integration**: Visual Studio and other IDE support

## 📖 Documentation

- **Learning Guide**: Step-by-step tutorial in `docs/learning_guide.md`
- **Installation Guide**: Boost setup instructions in `docs/boost_installation.md`
- **Code Comments**: Extensive inline documentation
- **Project READMEs**: Specific documentation for each project

## 🔍 Troubleshooting

### Common Issues

**Boost Not Found**
```
Solution: Install Boost using vcpkg or manual installation
See: docs/boost_installation.md
```

**Build Errors**
```
Solution: Ensure C++17 compiler and CMake 3.15+
Check: Visual Studio 2019+ or GCC 8+
```

**Runtime Errors**
```
Solution: Check firewall settings for networking examples
Ensure: Proper port permissions
```

## 🚀 Next Steps

1. **Install Boost**: Follow `docs/boost_installation.md`
2. **Build Project**: Run the build scripts
3. **Start Learning**: Begin with `docs/learning_guide.md`
4. **Practice**: Work through examples progressively
5. **Build Projects**: Implement the chat server
6. **Extend**: Create your own networking applications

## 🌟 Key Features

- ✅ **Complete Independence**: No external project dependencies
- ✅ **Self-Contained**: All necessary files included
- ✅ **Progressive Learning**: Structured from basics to advanced
- ✅ **Practical Projects**: Real-world applications
- ✅ **Cross-Platform**: Windows and Linux support
- ✅ **Modern C++**: C++17 standards and best practices
- ✅ **Comprehensive Documentation**: Detailed guides and examples
- ✅ **Automated Building**: One-command build process

---

**Ready to master Boost.Asio networking? Start with the learning guide and build your first asynchronous application!**