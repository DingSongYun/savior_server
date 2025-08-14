# SaviorServer 代码风格规范

## 概述

本项目采用 **Microsoft C++ Style** 代码风格，以确保代码的一致性和可读性。

## 命名规范

### 1. 类名 (Classes)
- 使用 **PascalCase**
- 示例：`TcpServer`, `MessageFactory`, `ConnectionManager`

### 2. 函数名 (Functions/Methods)
- 使用 **PascalCase**
- 示例：`SendMessage()`, `Initialize()`, `GetConnectionId()`

### 3. 变量名 (Variables)
- 使用 **camelCase**
- 示例：`connectionId`, `messageBuffer`, `serverPort`

### 4. 常量 (Constants)
- 使用 **PascalCase**
- 示例：`MaxConnections`, `DefaultPort`, `BufferSize`

### 5. 成员变量 (Member Variables)
- 使用 **m_** 前缀 + **camelCase**
- 示例：`m_connectionId`, `m_isRunning`, `m_messageQueue`

### 6. 文件名 (File Names)
- 使用 **PascalCase**
- 头文件使用 `.hpp` 扩展名
- 源文件使用 `.cpp` 扩展名
- 示例：`TcpServer.hpp`, `MessageTypes.cpp`

### 7. 命名空间 (Namespaces)
- 使用 **snake_case**
- 示例：`savior_server::network`, `savior_server::core`

### 8. 枚举 (Enums)
- 枚举类型使用 **PascalCase**
- 枚举值使用 **PascalCase**
- 示例：
```cpp
enum class MessageType {
    Text,
    Heartbeat,
    Connection,
    Disconnection
};
```

## 代码格式

### 1. 缩进
- 使用 **4个空格** 进行缩进
- 不使用制表符 (Tab)

### 2. 大括号
- 使用 **Allman 风格**（大括号独占一行）
```cpp
class TcpServer
{
public:
    void Initialize()
    {
        // 实现代码
    }
};
```

### 3. 行长度
- 每行最多 **120个字符**
- 超过时适当换行

### 4. 空行
- 类定义之间空一行
- 函数定义之间空一行
- 逻辑块之间适当空行

## 注释规范

### 1. 文件头注释
```cpp
/**
 * @file TcpServer.hpp
 * @brief TCP服务器类定义
 * @author SaviorServer Team
 * @date 2024-01-XX
 */
```

### 2. 类注释
```cpp
/**
 * @brief TCP服务器类，负责处理网络连接和消息传输
 */
class TcpServer
{
    // ...
};
```

### 3. 函数注释
```cpp
/**
 * @brief 发送消息到指定连接
 * @param connectionId 连接ID
 * @param message 要发送的消息
 * @return 发送是否成功
 */
bool SendMessage(int connectionId, const std::string& message);
```

### 4. 行内注释
- 使用 `//` 进行行内注释
- 注释应该解释 "为什么" 而不是 "是什么"

## 示例代码

```cpp
/**
 * @file TcpServer.hpp
 * @brief TCP服务器类定义
 */

#pragma once

#include <string>
#include <memory>

namespace savior_server::network
{
    /**
     * @brief TCP服务器类
     */
    class TcpServer
    {
    public:
        /**
         * @brief 构造函数
         * @param port 监听端口
         */
        explicit TcpServer(int port);
        
        /**
         * @brief 析构函数
         */
        ~TcpServer();
        
        /**
         * @brief 初始化服务器
         * @return 初始化是否成功
         */
        bool Initialize();
        
        /**
         * @brief 启动服务器
         * @return 启动是否成功
         */
        bool Start();
        
        /**
         * @brief 停止服务器
         */
        void Stop();
        
    private:
        int m_port;                    ///< 监听端口
        bool m_isRunning;              ///< 运行状态
        std::string m_serverAddress;   ///< 服务器地址
        
        /**
         * @brief 处理新连接
         */
        void HandleNewConnection();
    };
}
```

## 重构计划

当前项目中存在多种命名风格混用的情况，需要按以下优先级进行重构：

1. **高优先级**：公共接口（头文件中的类名、公共方法名）
2. **中优先级**：实现文件中的方法名和成员变量
3. **低优先级**：局部变量和私有方法

## 工具支持

建议使用以下工具来保持代码风格一致性：
- **clang-format**：自动格式化代码
- **Visual Studio Code Settings**：配置编辑器格式化规则
- **Git Hooks**：提交前自动检查代码风格

---

*本文档将随着项目发展持续更新和完善。*