#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

using namespace boost;
using namespace std::chrono_literals;

// 1. 传统阻塞IO模型演示
void demonstrate_blocking_io() {
    std::cout << "\n=== 1. 传统阻塞IO模型 ===" << std::endl;
    std::cout << "问题：每个连接需要一个线程，资源消耗大" << std::endl;
    
    // 模拟传统阻塞IO处理多个客户端
    std::vector<std::thread> client_threads;
    
    for (int i = 0; i < 3; ++i) {
        client_threads.emplace_back([i]() {
            std::cout << "客户端 " << i << " 连接，分配线程 " 
                      << std::this_thread::get_id() << std::endl;
            
            // 模拟阻塞读取
            std::this_thread::sleep_for(200ms);
            std::cout << "客户端 " << i << " 数据处理完成" << std::endl;
        });
    }
    
    for (auto& t : client_threads) {
        t.join();
    }
    
    std::cout << "传统模型：3个客户端 = 3个线程" << std::endl;
}

// 2. IO多路复用模型演示
void demonstrate_io_multiplexing() {
    std::cout << "\n=== 2. IO多路复用模型 ===" << std::endl;
    std::cout << "优势：单线程处理多个连接，高效利用资源" << std::endl;
    
    asio::io_context io;
    std::vector<std::unique_ptr<asio::steady_timer>> clients;
    
    // 模拟多个客户端连接
    for (int i = 0; i < 10; ++i) {
        auto client = std::make_unique<asio::steady_timer>(io, 
            std::chrono::milliseconds(100 + i * 50));
        
        client->async_wait([i](const boost::system::error_code& ec) {
            std::cout << "客户端 " << i << " 数据就绪，线程: " 
                      << std::this_thread::get_id() << std::endl;
        });
        
        clients.push_back(std::move(client));
    }
    
    std::cout << "单线程处理 " << clients.size() << " 个客户端连接" << std::endl;
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;
    
    io.run();
    std::cout << "IO多路复用：10个客户端 = 1个线程" << std::endl;
}

// 3. select/poll/epoll 机制对比
void demonstrate_multiplexing_mechanisms() {
    std::cout << "\n=== 3. 不同多路复用机制对比 ===" << std::endl;
    
    std::cout << "\n--- select 机制模拟 ---" << std::endl;
    std::cout << "特点：使用fd_set，有1024个文件描述符限制" << std::endl;
    
    asio::io_context select_io;
    const int select_limit = 5; // 模拟select的限制
    
    for (int i = 0; i < select_limit; ++i) {
        auto timer = std::make_shared<asio::steady_timer>(select_io, 100ms);
        timer->async_wait([i, timer](const boost::system::error_code& ec) {
            std::cout << "select: 处理fd " << i << std::endl;
        });
    }
    
    select_io.run();
    
    std::cout << "\n--- epoll 机制模拟 ---" << std::endl;
    std::cout << "特点：无文件描述符限制，边缘触发，高性能" << std::endl;
    
    asio::io_context epoll_io;
    
    std::vector<std::shared_ptr<asio::steady_timer>> epoll_timers;
    for (int i = 0; i < 20; ++i) { // 模拟大量连接
        auto timer = std::make_shared<asio::steady_timer>(epoll_io, 
            std::chrono::milliseconds(50 + i * 10));
        
        timer->async_wait([i, timer](const boost::system::error_code& ec) {
            std::cout << "epoll: 高效处理连接 " << i << std::endl;
        });
        
        epoll_timers.push_back(timer);
    }
    
    epoll_io.run();
}

// 4. 事件驱动模型
void demonstrate_event_driven_model() {
    std::cout << "\n=== 4. 事件驱动模型 ===" << std::endl;
    
    asio::io_context io;
    std::atomic<int> event_count{0};
    
    // 模拟不同类型的IO事件
    std::vector<std::string> event_types = {
        "网络读事件", "网络写事件", "定时器事件", 
        "信号事件", "文件IO事件"
    };
    
    for (size_t i = 0; i < event_types.size(); ++i) {
        auto timer = std::make_shared<asio::steady_timer>(io, 
            std::chrono::milliseconds(100 + i * 50));
        
        timer->async_wait([i, &event_types, &event_count, timer]
                         (const boost::system::error_code& ec) {
            event_count++;
            std::cout << "事件 " << event_count << ": " 
                      << event_types[i] << " 触发" << std::endl;
        });
    }
    
    std::cout << "事件循环开始，等待IO事件..." << std::endl;
    io.run();
    std::cout << "总共处理了 " << event_count << " 个IO事件" << std::endl;
}

// 5. Reactor模式演示
void demonstrate_reactor_pattern() {
    std::cout << "\n=== 5. Reactor模式演示 ===" << std::endl;
    std::cout << "Reactor = 事件分离器 + 事件处理器" << std::endl;
    
    asio::io_context reactor; // 事件分离器
    
    // 注册不同的事件处理器
    std::cout << "\n注册事件处理器..." << std::endl;
    
    // 处理器1：连接事件
    auto connection_timer = std::make_shared<asio::steady_timer>(reactor, 100ms);
    connection_timer->async_wait([connection_timer](const boost::system::error_code& ec) {
        std::cout << "连接处理器：新客户端连接" << std::endl;
    });
    
    // 处理器2：读事件
    auto read_timer = std::make_shared<asio::steady_timer>(reactor, 200ms);
    read_timer->async_wait([read_timer](const boost::system::error_code& ec) {
        std::cout << "读处理器：接收客户端数据" << std::endl;
    });
    
    // 处理器3：写事件
    auto write_timer = std::make_shared<asio::steady_timer>(reactor, 300ms);
    write_timer->async_wait([write_timer](const boost::system::error_code& ec) {
        std::cout << "写处理器：发送响应数据" << std::endl;
    });
    
    // 处理器4：断开事件
    auto disconnect_timer = std::make_shared<asio::steady_timer>(reactor, 400ms);
    disconnect_timer->async_wait([disconnect_timer](const boost::system::error_code& ec) {
        std::cout << "断开处理器：客户端断开连接" << std::endl;
    });
    
    std::cout << "Reactor开始事件循环..." << std::endl;
    reactor.run();
    std::cout << "Reactor模式演示完成" << std::endl;
}

// 6. 性能对比分析
void demonstrate_performance_comparison() {
    std::cout << "\n=== 6. 性能对比分析 ===" << std::endl;
    
    const int num_connections = 1000;
    
    std::cout << "\n--- 传统多线程模型 ---" << std::endl;
    
    // 模拟传统模型的资源消耗
    std::cout << "创建 " << num_connections << " 个线程..." << std::endl;
    std::cout << "内存消耗：" << num_connections << " x 8MB = " 
              << (num_connections * 8) << "MB" << std::endl;
    std::cout << "上下文切换开销：高" << std::endl;
    
    std::cout << "\n--- IO多路复用模型 ---" << std::endl;
    
    asio::io_context io;
    std::vector<std::shared_ptr<asio::steady_timer>> connections;
    
    for (int i = 0; i < 100; ++i) { // 模拟部分连接
        auto conn = std::make_shared<asio::steady_timer>(io, 1ms);
        conn->async_wait([i, conn](const boost::system::error_code& ec) {
            // 模拟处理
        });
        connections.push_back(conn);
    }
    
    io.run();
    
    std::cout << "处理 " << connections.size() << " 个连接" << std::endl;
    std::cout << "内存消耗：1个线程约8MB" << std::endl;
    std::cout << "上下文切换开销：无" << std::endl;
    
    std::cout << "\n性能优势：" << std::endl;
    std::cout << "- 内存节省：" << ((num_connections * 8) - 8) << "MB" << std::endl;
    std::cout << "- CPU效率：避免线程切换开销" << std::endl;
    std::cout << "- 可扩展性：支持数万并发连接" << std::endl;
}

// 7. 底层实现机制
void demonstrate_underlying_mechanisms() {
    std::cout << "\n=== 7. 底层实现机制 ===" << std::endl;
    
    std::cout << "\n--- Linux epoll ---" << std::endl;
    std::cout << "1. epoll_create(): 创建epoll实例" << std::endl;
    std::cout << "2. epoll_ctl(): 添加/删除/修改监听的文件描述符" << std::endl;
    std::cout << "3. epoll_wait(): 等待IO事件发生" << std::endl;
    std::cout << "特点：边缘触发(ET)和水平触发(LT)" << std::endl;
    
    std::cout << "\n--- Windows IOCP ---" << std::endl;
    std::cout << "1. CreateIoCompletionPort(): 创建完成端口" << std::endl;
    std::cout << "2. 绑定socket到完成端口" << std::endl;
    std::cout << "3. GetQueuedCompletionStatus(): 获取完成事件" << std::endl;
    std::cout << "特点：真正的异步IO，零拷贝" << std::endl;
    
    std::cout << "\n--- macOS kqueue ---" << std::endl;
    std::cout << "1. kqueue(): 创建内核事件队列" << std::endl;
    std::cout << "2. kevent(): 注册事件和获取事件" << std::endl;
    std::cout << "特点：统一的事件通知机制" << std::endl;
    
    // Asio自动选择最优机制
    asio::io_context io;
    std::cout << "\nBoost.Asio 自动选择最优机制：" << std::endl;
#ifdef _WIN32
    std::cout << "当前平台：Windows，使用 IOCP" << std::endl;
#elif defined(__linux__)
    std::cout << "当前平台：Linux，使用 epoll" << std::endl;
#elif defined(__APPLE__)
    std::cout << "当前平台：macOS，使用 kqueue" << std::endl;
#else
    std::cout << "当前平台：其他，使用 select/poll" << std::endl;
#endif
}

int main() {
    std::cout << "IO多路复用机制深度解析" << std::endl;
    std::cout << "========================" << std::endl;
    
    demonstrate_blocking_io();
    demonstrate_io_multiplexing();
    demonstrate_multiplexing_mechanisms();
    demonstrate_event_driven_model();
    demonstrate_reactor_pattern();
    demonstrate_performance_comparison();
    demonstrate_underlying_mechanisms();
    
    std::cout << "\n=== IO多路复用总结 ===" << std::endl;
    std::cout << "1. 定义：单线程监控多个IO通道，哪个就绪就处理哪个" << std::endl;
    std::cout << "2. 核心：避免阻塞等待，提高CPU利用率" << std::endl;
    std::cout << "3. 实现：select/poll/epoll/kqueue/IOCP" << std::endl;
    std::cout << "4. 优势：高并发、低资源消耗、高性能" << std::endl;
    std::cout << "5. 应用：Web服务器、游戏服务器、代理服务器" << std::endl;
    
    return 0;
}