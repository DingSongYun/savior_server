# Examples Directory

This directory contains learning examples and projects for the Sarior Server ecosystem.

## 📁 Structure

```
examples/
├── README.md                           # This file
├── boost_asio_learning/               # Complete Boost.Asio learning project
│   ├── README.md                      # Project overview and quick start
│   ├── CMakeLists.txt                 # Independent build configuration
│   ├── docs/                          # Comprehensive documentation
│   │   ├── learning_guide.md          # Step-by-step learning guide
│   │   └── boost_installation.md      # Boost installation instructions
│   ├── basic_examples/                # Progressive learning examples
│   │   ├── 01_basic_concepts.cpp      # io_context, async basics
│   │   ├── 02_timers.cpp             # Asynchronous timers
│   │   ├── 03_networking_basics.cpp   # TCP/UDP networking
│   │   ├── 04_advanced_networking.cpp # Advanced networking features
│   │   └── 05_strand_thread_safety.cpp # Thread safety with strands
│   ├── projects/                      # Practical projects
│   │   └── chat_server/              # Multi-client chat server
│   │       ├── chat_server.cpp       # Server implementation
│   │       ├── chat_client.cpp       # Client implementation
│   │       └── README.md             # Project documentation
│   ├── build/                        # Build scripts
│   │   ├── build.bat                 # Windows build script
│   │   └── build.sh                  # Linux build script
│   └── tools/                        # Utility scripts
│       ├── setup_env.bat             # Environment setup
│       └── test_runner.py            # Test automation
└── [Legacy files - to be cleaned up]
```

## 🚀 Quick Start with Boost.Asio Learning

The main learning project is now located in `boost_asio_learning/` directory:

```bash
# Navigate to the learning project
cd examples/boost_asio_learning

# Read the project documentation
# - README.md for overview and quick start
# - docs/learning_guide.md for detailed learning path
# - docs/boost_installation.md for Boost setup

# Build and run (Windows)
cd build
.\build.bat

# Build and run (Linux)
cd build
./build.sh
```

## ✨ Key Features

- **🔗 Complete Independence**: No dependencies on parent mobileserver project
- **📚 Self-Contained Learning**: All materials included in one place
- **🎯 Progressive Structure**: From basic concepts to advanced projects
- **🛠️ Automated Building**: One-command build process
- **🌐 Cross-Platform**: Windows and Linux support
- **📖 Comprehensive Docs**: Detailed guides and examples

## 📋 Learning Path

1. **Start Here**: `boost_asio_learning/README.md`
2. **Setup Environment**: `boost_asio_learning/docs/boost_installation.md`
3. **Learn Progressively**: `boost_asio_learning/docs/learning_guide.md`
4. **Practice Examples**: Work through `basic_examples/` in order
5. **Build Projects**: Implement `projects/chat_server/`
6. **Extend Further**: Create your own networking applications

## 🎯 Project Goals

The Boost.Asio learning project is designed to:
- Master asynchronous programming concepts
- Understand modern C++ networking patterns
- Build real-world networking applications
- Learn performance optimization techniques
- Develop cross-platform networking skills

---

**Ready to start learning? Head to `boost_asio_learning/` and begin your networking journey!**

## 目录结构

```
asio_examples/
├── 01_basic_concepts.cpp      # 基础概念：io_context, work_guard, post/dispatch
├── 02_timers.cpp              # 定时器：基本用法、重复定时器、任务调度
├── 03_networking_basics.cpp   # 网络编程基础：TCP/UDP 客户端和服务器
├── 04_advanced_networking.cpp # 高级网络编程：连接池、HTTP客户端、多线程服务器
├── 05_strand_thread_safety.cpp# Strand 和线程安全
└── README.md                  # 本文件
```

## 编译要求

### Windows (Visual Studio)
确保已安装：
- Visual Studio 2019 或更新版本
- Boost 库 (推荐 1.70 或更新版本)

### 编译命令

#### 使用 g++ (MinGW/MSYS2)
```bash
# 基础示例
g++ -std=c++17 01_basic_concepts.cpp -lboost_system -pthread -o 01_basic_concepts.exe

# 定时器示例
g++ -std=c++17 02_timers.cpp -lboost_system -pthread -o 02_timers.exe

# 网络编程基础
g++ -std=c++17 03_networking_basics.cpp -lboost_system -pthread -o 03_networking_basics.exe

# 高级网络编程
g++ -std=c++17 04_advanced_networking.cpp -lboost_system -pthread -o 04_advanced_networking.exe

# Strand 和线程安全
g++ -std=c++17 05_strand_thread_safety.cpp -lboost_system -pthread -o 05_strand_thread_safety.exe
```

#### 使用 Visual Studio (cl.exe)
```cmd
# 首先设置 Visual Studio 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# 编译示例（需要配置 Boost 路径）
cl /EHsc /std:c++17 01_basic_concepts.cpp /I"C:\boost\include" /link /LIBPATH:"C:\boost\lib"
```

## 运行示例

### 1. 基础概念 (01_basic_concepts.cpp)
```bash
./01_basic_concepts.exe
```
学习内容：
- `io_context` 的基本使用
- `work_guard` 保持 `io_context` 运行
- `post` 和 `dispatch` 的区别
- 多线程环境下的 `io_context`
- 错误处理

### 2. 定时器 (02_timers.cpp)
```bash
./02_timers.exe
```
学习内容：
- `steady_timer` 基本用法
- 重复定时器实现
- 多个定时器管理
- 定时器取消
- 简单任务调度器

### 3. 网络编程基础 (03_networking_basics.cpp)
```bash
./03_networking_basics.exe
```
学习内容：
- TCP 客户端和服务器
- UDP 客户端和服务器
- 异步连接和数据传输
- 域名解析
- Echo 服务器实现

**注意**: 网络示例需要网络权限，某些防火墙可能会阻止连接。

### 4. 高级网络编程 (04_advanced_networking.cpp)
```bash
./04_advanced_networking.exe
```
学习内容：
- 连接池管理
- HTTP 客户端实现
- 多线程服务器
- 带超时的操作
- 简单负载均衡器

### 5. Strand 和线程安全 (05_strand_thread_safety.cpp)
```bash
./05_strand_thread_safety.exe
```
学习内容：
- `strand` 基本概念和使用
- 多 `strand` 并发处理
- `strand.wrap()` 函数包装
- 网络连接中的 `strand` 应用
- 有无 `strand` 的对比
- 自定义 `strand` 管理器

## 学习建议

### 学习顺序
1. **01_basic_concepts.cpp** - 理解 Boost.Asio 的核心概念
2. **02_timers.cpp** - 掌握异步定时器的使用
3. **03_networking_basics.cpp** - 学习基础网络编程
4. **05_strand_thread_safety.cpp** - 理解线程安全和 strand
5. **04_advanced_networking.cpp** - 掌握高级网络编程技巧

### 实践建议
1. **逐个运行示例**：每个示例都包含多个子功能，建议逐一测试
2. **修改参数**：尝试修改端口号、超时时间、线程数等参数
3. **添加日志**：在关键位置添加更多日志输出，观察程序执行流程
4. **错误处理**：故意制造错误（如连接不存在的服务器），观察错误处理
5. **性能测试**：使用多个客户端连接测试服务器性能

### 调试技巧
1. **使用调试器**：在 Visual Studio 中设置断点调试
2. **添加线程ID输出**：观察多线程执行情况
3. **网络工具**：使用 telnet、nc 等工具测试网络服务
4. **资源监控**：观察内存和CPU使用情况

## 常见问题

### 编译问题
1. **找不到 Boost 库**：
   - 确保正确安装 Boost
   - 检查编译器的包含路径和库路径

2. **链接错误**：
   - 确保链接了正确的 Boost 库（boost_system）
   - 在 Windows 上可能需要链接 ws2_32.lib

### 运行问题
1. **网络连接失败**：
   - 检查防火墙设置
   - 确保目标服务器可访问
   - 检查端口是否被占用

2. **程序卡住**：
   - 检查是否有 `work_guard` 阻止 `io_context` 退出
   - 确保所有异步操作都有适当的完成处理

### 性能优化
1. **线程数量**：根据 CPU 核心数调整线程池大小
2. **缓冲区大小**：根据数据量调整缓冲区大小
3. **连接池**：对于频繁的网络操作，使用连接池

## 进阶学习

完成这些示例后，建议继续学习：
1. **SSL/TLS 支持**：使用 `boost::asio::ssl`
2. **协程支持**：C++20 协程或 Boost.Coroutine
3. **序列化**：结合 Boost.Serialization
4. **WebSocket**：实现 WebSocket 协议
5. **HTTP 服务器**：完整的 HTTP 服务器实现

## 参考资源

- [Boost.Asio 官方文档](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [Boost.Asio 教程](https://www.boost.org/doc/libs/release/doc/html/boost_asio/tutorial.html)
- [Boost.Asio 示例](https://www.boost.org/doc/libs/release/doc/html/boost_asio/examples.html)