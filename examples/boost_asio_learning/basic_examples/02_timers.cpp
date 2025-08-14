// Boost.Asio 学习示例 - 第二阶段：定时器
// 编译命令: g++ -std=c++17 02_timers.cpp -lboost_system -pthread

#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <functional>

namespace asio = boost::asio;
using namespace std::chrono_literals;

// 示例1: 基本定时器使用
void example1_basic_timer() {
    std::cout << "\n=== 示例1: 基本定时器使用 ===" << std::endl;
    
    asio::io_context io;
    asio::steady_timer timer(io, 2s);  // 2秒后触发
    
    std::cout << "设置定时器，2秒后触发..." << std::endl;
    
    timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "定时器触发！" << std::endl;
        } else {
            std::cout << "定时器错误: " << ec.message() << std::endl;
        }
    });
    
    io.run();
}

// 示例2: 重复定时器
class RepeatingTimer {
private:
    asio::steady_timer timer_;
    int count_;
    int max_count_;
    
public:
    RepeatingTimer(asio::io_context& io, int max_count = 5) 
        : timer_(io), count_(0), max_count_(max_count) {}
    
    void start() {
        schedule_next();
    }
    
private:
    void schedule_next() {
        if (count_ >= max_count_) {
            std::cout << "重复定时器完成，共执行 " << count_ << " 次" << std::endl;
            return;
        }
        
        timer_.expires_after(1s);
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                ++count_;
                std::cout << "重复定时器第 " << count_ << " 次触发" << std::endl;
                schedule_next();  // 递归调度下一次
            }
        });
    }
};

void example2_repeating_timer() {
    std::cout << "\n=== 示例2: 重复定时器 ===" << std::endl;
    
    asio::io_context io;
    RepeatingTimer repeating_timer(io, 3);
    
    repeating_timer.start();
    io.run();
}

// 示例3: 多个定时器
void example3_multiple_timers() {
    std::cout << "\n=== 示例3: 多个定时器 ===" << std::endl;
    
    asio::io_context io;
    
    // 短定时器
    asio::steady_timer short_timer(io, 1s);
    short_timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "短定时器触发 (1秒)" << std::endl;
        }
    });
    
    // 中定时器
    asio::steady_timer medium_timer(io, 2s);
    medium_timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "中定时器触发 (2秒)" << std::endl;
        }
    });
    
    // 长定时器
    asio::steady_timer long_timer(io, 3s);
    long_timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "长定时器触发 (3秒)" << std::endl;
        }
    });
    
    io.run();
}

// 示例4: 定时器取消
void example4_timer_cancellation() {
    std::cout << "\n=== 示例4: 定时器取消 ===" << std::endl;
    
    asio::io_context io;
    
    asio::steady_timer timer1(io, 5s);
    asio::steady_timer timer2(io, 2s);
    
    // 长定时器
    timer1.async_wait([](const boost::system::error_code& ec) {
        if (ec == asio::error::operation_aborted) {
            std::cout << "长定时器被取消" << std::endl;
        } else if (!ec) {
            std::cout << "长定时器正常触发" << std::endl;
        }
    });
    
    // 短定时器，用于取消长定时器
    timer2.async_wait([&timer1](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "短定时器触发，取消长定时器" << std::endl;
            timer1.cancel();  // 取消长定时器
        }
    });
    
    io.run();
}

// 示例5: 任务调度器
class TaskScheduler {
private:
    asio::io_context& io_;
    asio::steady_timer timer_;
    
public:
    TaskScheduler(asio::io_context& io) : io_(io), timer_(io) {}
    
    // 延迟执行任务
    void schedule_task(std::chrono::seconds delay, std::function<void()> task) {
        auto shared_task = std::make_shared<std::function<void()>>(std::move(task));
        
        timer_.expires_after(delay);
        timer_.async_wait([shared_task](const boost::system::error_code& ec) {
            if (!ec && *shared_task) {
                (*shared_task)();
            }
        });
    }
    
    // 周期性执行任务
    void start_periodic_task(std::chrono::seconds interval, 
                           std::function<void()> task,
                           int max_executions = -1) {
        auto shared_data = std::make_shared<PeriodicTaskData>();
        shared_data->task = std::move(task);
        shared_data->interval = interval;
        shared_data->max_executions = max_executions;
        shared_data->current_execution = 0;
        shared_data->timer = std::make_unique<asio::steady_timer>(io_);
        
        schedule_periodic_execution(shared_data);
    }
    
private:
    struct PeriodicTaskData {
        std::function<void()> task;
        std::chrono::seconds interval;
        int max_executions;
        int current_execution;
        std::unique_ptr<asio::steady_timer> timer;
    };
    
    void schedule_periodic_execution(std::shared_ptr<PeriodicTaskData> data) {
        if (data->max_executions > 0 && data->current_execution >= data->max_executions) {
            return;  // 达到最大执行次数
        }
        
        data->timer->expires_after(data->interval);
        data->timer->async_wait([this, data](const boost::system::error_code& ec) {
            if (!ec) {
                data->current_execution++;
                if (data->task) {
                    data->task();
                }
                schedule_periodic_execution(data);  // 调度下一次执行
            }
        });
    }
};

void example5_task_scheduler() {
    std::cout << "\n=== 示例5: 任务调度器 ===" << std::endl;
    
    asio::io_context io;
    TaskScheduler scheduler(io);
    
    // 延迟任务
    scheduler.schedule_task(1s, []() {
        std::cout << "延迟任务执行 (1秒后)" << std::endl;
    });
    
    // 周期性任务
    scheduler.start_periodic_task(2s, []() {
        static int count = 0;
        std::cout << "周期性任务执行，第 " << ++count << " 次" << std::endl;
    }, 3);  // 最多执行3次
    
    // 设置一个定时器来停止 io_context
    asio::steady_timer stop_timer(io, 8s);
    stop_timer.async_wait([&io](const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "停止调度器" << std::endl;
            io.stop();
        }
    });
    
    io.run();
}

int main() {
    std::cout << "Boost.Asio 定时器学习示例" << std::endl;
    std::cout << "===========================" << std::endl;
    
    try {
        example1_basic_timer();
        example2_repeating_timer();
        example3_multiple_timers();
        example4_timer_cancellation();
        example5_task_scheduler();
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
    }
    
    std::cout << "\n所有定时器示例运行完成！" << std::endl;
    return 0;
}