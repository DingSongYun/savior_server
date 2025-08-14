// Boost.Asio 中的 IO 概念深度解析
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <functional>

namespace asio = boost::asio;
using namespace std::chrono_literals;

// 1. 传统同步 IO vs 异步 IO 对比
void demonstrate_sync_vs_async_io() {
    std::cout << "\n=== 1. 同步 IO vs 异步 IO 对比 ===" << std::endl;
    
    // 同步 IO 示例
    std::cout << "\n--- 同步 IO (阻塞) ---" << std::endl;
    auto start = std::chrono::steady_clock::now();
    
    std::cout << "开始同步读取..." << std::endl;
    std::this_thread::sleep_for(1s); // 模拟 IO 等待
    std::cout << "同步读取完成" << std::endl;
    
    auto sync_duration = std::chrono::steady_clock::now() - start;
    std::cout << "同步 IO 总耗时: " << 
        std::chrono::duration_cast<std::chrono::milliseconds>(sync_duration).count() 
        << "ms" << std::endl;
    
    // 异步 IO 示例
    std::cout << "\n--- 异步 IO (非阻塞) ---" << std::endl;
    start = std::chrono::steady_clock::now();
    
    asio::io_context io;
    
    asio::steady_timer timer1(io, 1s);
    timer1.async_wait([](const boost::system::error_code& ec) {
        std::cout << "异步操作1完成 (模拟文件读取)" << std::endl;
    });
    
    asio::steady_timer timer2(io, 500ms);
    timer2.async_wait([](const boost::system::error_code& ec) {
        std::cout << "异步操作2完成 (模拟网络请求)" << std::endl;
    });
    
    std::cout << "启动异步操作，继续执行其他任务..." << std::endl;
    
    for (int i = 0; i < 3; ++i) {
        std::cout << "主线程工作: " << i << std::endl;
        std::this_thread::sleep_for(200ms);
    }
    
    io.run();
    
    auto async_duration = std::chrono::steady_clock::now() - start;
    std::cout << "异步 IO 总耗时: " << 
        std::chrono::duration_cast<std::chrono::milliseconds>(async_duration).count() 
        << "ms" << std::endl;
}

// 2. IO 的不同类型
void demonstrate_io_types() {
    std::cout << "\n=== 2. IO 的不同类型 ===" << std::endl;
    
    asio::io_context io;
    
    std::cout << "\n--- 网络 IO ---" << std::endl;
    asio::steady_timer network_timer(io, 300ms);
    network_timer.async_wait([](const boost::system::error_code& ec) {
        std::cout << "网络 IO 完成: 接收到数据包" << std::endl;
    });
    
    std::cout << "\n--- 文件 IO ---" << std::endl;
    asio::steady_timer file_timer(io, 200ms);
    file_timer.async_wait([](const boost::system::error_code& ec) {
        std::cout << "文件 IO 完成: 文件读取完毕" << std::endl;
    });
    
    std::cout << "\n--- 定时器 IO ---" << std::endl;
    asio::steady_timer timer_io(io, 100ms);
    timer_io.async_wait([](const boost::system::error_code& ec) {
        std::cout << "定时器 IO 完成: 定时事件触发" << std::endl;
    });
    
    std::cout << "\n--- 信号 IO ---" << std::endl;
    asio::steady_timer signal_timer(io, 400ms);
    signal_timer.async_wait([](const boost::system::error_code& ec) {
        std::cout << "信号 IO 完成: 系统信号处理" << std::endl;
    });
    
    std::cout << "所有 IO 操作已启动，等待完成..." << std::endl;
    io.run();
}

// 3. IO 事件驱动模型
void demonstrate_event_driven_io() {
    std::cout << "\n=== 3. IO 事件驱动模型 ===" << std::endl;
    
    asio::io_context io;
    std::vector<std::unique_ptr<asio::steady_timer>> timers;
    
    for (int i = 0; i < 5; ++i) {
        auto timer = std::make_unique<asio::steady_timer>(io, 
            std::chrono::milliseconds(100 * (i + 1)));
        
        timer->async_wait([i](const boost::system::error_code& ec) {
            std::cout << "IO 事件 " << i << " 触发 (线程: " 
                      << std::this_thread::get_id() << ")" << std::endl;
        });
        
        timers.push_back(std::move(timer));
    }
    
    std::cout << "事件循环开始..." << std::endl;
    
    std::vector<std::thread> workers;
    for (int i = 0; i < 2; ++i) {
        workers.emplace_back([&io, i]() {
            std::cout << "工作线程 " << i << " 启动 (ID: " 
                      << std::this_thread::get_id() << ")" << std::endl;
            io.run();
        });
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "所有 IO 事件处理完成" << std::endl;
}

// 4. IO 完成模型
void demonstrate_io_completion_model() {
    std::cout << "\n=== 4. IO 完成模型 ===" << std::endl;
    
    asio::io_context io;
    const int num_operations = 10;
    std::atomic<int> completed_operations{0};
    
    for (int i = 0; i < num_operations; ++i) {
        auto timer = std::make_shared<asio::steady_timer>(io, 
            std::chrono::milliseconds(50 + (i % 3) * 100));
        
        timer->async_wait([i, &completed_operations, timer](const boost::system::error_code& ec) {
            std::cout << "IO 操作 " << i << " 完成" << std::endl;
            completed_operations++;
        });
    }
    
    std::cout << "启动 " << num_operations << " 个并发 IO 操作..." << std::endl;
    
    asio::steady_timer monitor(io, 50ms);
    std::function<void(const boost::system::error_code&)> check_progress;
    check_progress = [&](const boost::system::error_code& ec) {
        std::cout << "进度: " << completed_operations << "/" << num_operations << std::endl;
        
        if (completed_operations < num_operations) {
            monitor.expires_after(50ms);
            monitor.async_wait(check_progress);
        }
    };
    monitor.async_wait(check_progress);
    
    io.run();
    std::cout << "所有 IO 操作完成!" << std::endl;
}

// 5. io_context 的作用
void demonstrate_io_context_role() {
    std::cout << "\n=== 5. io_context 的作用 ===" << std::endl;
    
    std::cout << "\n--- io_context 作为事件循环 ---" << std::endl;
    asio::io_context io;
    
    asio::post(io, []() {
        std::cout << "任务1: 在事件循环中执行" << std::endl;
    });
    
    asio::post(io, []() {
        std::cout << "任务2: 在事件循环中执行" << std::endl;
    });
    
    std::cout << "\n--- io_context 作为调度器 ---" << std::endl;
    asio::steady_timer timer(io, 100ms);
    timer.async_wait([](const boost::system::error_code& ec) {
        std::cout << "定时任务: 由调度器在指定时间执行" << std::endl;
    });
    
    std::cout << "\n--- io_context 作为线程池管理器 ---" << std::endl;
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;
    
    std::thread worker([&io]() {
        std::cout << "工作线程 ID: " << std::this_thread::get_id() << std::endl;
        io.run();
    });
    
    worker.join();
}

// 6. IO 操作的生命周期
void demonstrate_io_lifecycle() {
    std::cout << "\n=== 6. IO 操作的生命周期 ===" << std::endl;
    
    asio::io_context io;
    
    std::cout << "1. 创建 IO 对象" << std::endl;
    auto timer = std::make_shared<asio::steady_timer>(io, 200ms);
    
    std::cout << "2. 启动异步操作" << std::endl;
    timer->async_wait([timer](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "4. IO 操作成功完成" << std::endl;
        } else {
            std::cout << "4. IO 操作失败: " << ec.message() << std::endl;
        }
        std::cout << "5. 清理资源 (智能指针自动管理)" << std::endl;
    });
    
    std::cout << "3. 等待 IO 完成..." << std::endl;
    io.run();
    std::cout << "6. 事件循环结束" << std::endl;
}

int main() {
    std::cout << "Boost.Asio 中的 IO 概念深度解析" << std::endl;
    std::cout << "=================================" << std::endl;
    
    demonstrate_sync_vs_async_io();
    demonstrate_io_types();
    demonstrate_event_driven_io();
    demonstrate_io_completion_model();
    demonstrate_io_context_role();
    demonstrate_io_lifecycle();
    
    std::cout << "\n=== IO 概念总结 ===" << std::endl;
    std::cout << "1. IO = Input/Output，包括网络、文件、定时器、信号等" << std::endl;
    std::cout << "2. 异步 IO 允许非阻塞操作，提高程序并发性" << std::endl;
    std::cout << "3. io_context 是事件循环、调度器和线程池管理器" << std::endl;
    std::cout << "4. 事件驱动模型通过回调处理 IO 完成事件" << std::endl;
    std::cout << "5. Asio 抽象了底层 IO 复用机制 (epoll/IOCP)" << std::endl;
    
    return 0;
}