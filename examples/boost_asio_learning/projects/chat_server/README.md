# 聊天服务器项目

这是一个基于 Boost.Asio 的多客户端聊天服务器实现，展示了异步网络编程的核心概念。

## 项目特点

- **异步 I/O**: 使用 Boost.Asio 的异步模型
- **多客户端支持**: 支持多个客户端同时连接
- **线程安全**: 使用 `strand` 确保线程安全
- **消息广播**: 支持消息广播到所有客户端
- **房间管理**: 支持多个聊天房间
- **消息历史**: 保存聊天历史记录

## 文件说明

- `chat_server.cpp` - 聊天服务器实现
- `chat_client.cpp` - 聊天客户端实现

## 编译和运行

### 使用构建脚本
```bash
# Windows
cd ..\build
build.bat

# Linux
cd ../build
chmod +x build.sh
./build.sh
```

### 手动编译
```bash
# Windows (Visual Studio)
cl /EHsc chat_server.cpp /I"C:\boost\include" /link /LIBPATH:"C:\boost\lib" ws2_32.lib

# Linux (g++)
g++ -std=c++17 -o chat_server chat_server.cpp -lboost_system -pthread
g++ -std=c++17 -o chat_client chat_client.cpp -lboost_system -pthread
```

## 使用方法

### 1. 启动服务器
```bash
./chat_server
# 服务器将在端口 8888 上监听
```

### 2. 启动客户端
```bash
./chat_client
# 按提示输入服务器地址和端口
```

### 3. 聊天命令
客户端支持以下命令：
- `/join <room>` - 加入聊天房间
- `/leave` - 离开当前房间
- `/list` - 列出所有房间
- `/users` - 列出当前房间用户
- `/quit` - 退出客户端
- 其他输入作为聊天消息发送

## 技术要点

### 服务器端
1. **异步接受连接**
   ```cpp
   void start_accept() {
       auto new_client = std::make_shared<ChatClient>(io_context_, *this);
       acceptor_.async_accept(new_client->socket(),
           [this, new_client](std::error_code ec) {
               if (!ec) {
                   new_client->start();
                   start_accept();
               }
           });
   }
   ```

2. **使用 Strand 保证线程安全**
   ```cpp
   class ChatClient {
       boost::asio::strand<boost::asio::io_context::executor_type> strand_;
   public:
       ChatClient(boost::asio::io_context& io_context, ChatServer& server)
           : socket_(io_context), server_(server), 
             strand_(boost::asio::make_strand(io_context)) {}
   };
   ```

3. **异步读写操作**
   ```cpp
   void do_read() {
       auto self(shared_from_this());
       boost::asio::async_read_until(socket_, buffer_, '\n',
           boost::asio::bind_executor(strand_,
               [this, self](std::error_code ec, std::size_t length) {
                   if (!ec) {
                       handle_message();
                       do_read();
                   }
               }));
   }
   ```

### 客户端
1. **异步连接**
   ```cpp
   void connect(const std::string& host, const std::string& port) {
       boost::asio::ip::tcp::resolver resolver(io_context_);
       auto endpoints = resolver.resolve(host, port);
       
       boost::asio::async_connect(socket_, endpoints,
           [this](std::error_code ec, boost::asio::ip::tcp::endpoint) {
               if (!ec) {
                   start_read();
               }
           });
   }
   ```

2. **双向通信**
   - 一个线程处理用户输入
   - 异步处理服务器消息

## 学习要点

1. **异步编程模型**: 理解回调函数和异步操作链
2. **内存管理**: 使用 `shared_ptr` 管理对象生命周期
3. **线程安全**: 使用 `strand` 序列化操作
4. **错误处理**: 正确处理网络错误和异常
5. **协议设计**: 简单的文本协议实现

## 扩展建议

1. **安全性**: 添加用户认证和授权
2. **持久化**: 将聊天记录保存到数据库
3. **协议**: 使用二进制协议提高效率
4. **功能**: 添加私聊、文件传输等功能
5. **监控**: 添加连接状态监控和日志

## 常见问题

### Q: 服务器无法启动
A: 检查端口是否被占用，可以修改代码中的端口号

### Q: 客户端连接失败
A: 确认服务器已启动，检查防火墙设置

### Q: 消息乱序
A: 这是正常现象，可以添加消息序号来保证顺序

### Q: 内存泄漏
A: 确保正确使用 `shared_ptr` 和 `weak_ptr`