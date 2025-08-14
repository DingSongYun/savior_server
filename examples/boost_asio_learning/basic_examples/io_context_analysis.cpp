// Boost.Asio io_context 深度分析示例
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

namespace asio = boost::asio;

void analyze_basic_lifecycle() {
    std::cout << "\n=== Analysis 1: io_context Basic Lifecycle ===" << std::endl;
    
    asio::io_context io;
    
    std::cout << "1. io_context created" << std::endl;
    std::cout << "   - stopped(): " << io.stopped() << std::endl;
    
    asio::post(io, []() {
        std::cout << "   - Task executing..." << std::endl;
    });
    
    std::cout << "2. Task posted, ready to run" << std::endl;
    std::cout << "   - stopped(): " << io.stopped() << std::endl;
    
    std::cout << "3. Starting io.run()" << std::endl;
    size_t handlers_run = io.run();
    
    std::cout << "4. io.run() completed" << std::endl;
    std::cout << "   - Handlers executed: " << handlers_run << std::endl;
    std::cout << "   - stopped(): " << io.stopped() << std::endl;
    
    std::cout << "5. Restarting io_context" << std::endl;
    io.restart();
    std::cout << "   - stopped(): " << io.stopped() << std::endl;

    std::cout << "6. run after restart io_context" << std::endl;
    handlers_run = io.run();
    std::cout << "   - Handlers executed: " << handlers_run << std::endl;
    std::cout << "   - stopped(): " << io.stopped() << std::endl;

}

void analyze_run_methods() {
    std::cout << "\n=== Analysis 2: Different Run Methods ===" << std::endl;
    
    // run() method
    std::cout << "\n--- run() method ---" << std::endl;
    asio::io_context io1;
    
    for (int i = 0; i < 3; ++i) {
        asio::post(io1, [i]() {
            std::cout << "run() - Task " << i << std::endl;
        });
    }
    
    size_t count = io1.run();
    std::cout << "run() executed " << count << " handlers" << std::endl;
    
    // run_one() method
    std::cout << "\n--- run_one() method ---" << std::endl;
    asio::io_context io2;
    
    for (int i = 0; i < 3; ++i) {
        asio::post(io2, [i]() {
            std::cout << "run_one() - Task " << i << std::endl;
        });
    }
    
    while (!io2.stopped()) {
        size_t count = io2.run_one();
        std::cout << "run_one() executed " << count << " handler" << std::endl;
        if (count == 0) break;
    }
    
    // poll() method
    std::cout << "\n--- poll() method ---" << std::endl;
    asio::io_context io3;
    
    for (int i = 0; i < 2; ++i) {
        asio::post(io3, [i]() {
            std::cout << "poll() - Task " << i << std::endl;
        });
    }
    
    std::cout << "First poll(): ";
    size_t count1 = io3.poll();
    std::cout << count1 << " handlers" << std::endl;
    
    std::cout << "Second poll(): ";
    size_t count2 = io3.poll();
    std::cout << count2 << " handlers" << std::endl;
}

void analyze_multithreading() {
    std::cout << "\n=== Analysis 3: Multithreading Environment ===" << std::endl;
    
    asio::io_context io;
    std::atomic<int> completed_tasks{0};
    
    for (int i = 0; i < 10; ++i) {
        asio::post(io, [i, &completed_tasks]() {
            std::cout << "Task " << i << " executing in thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            completed_tasks++;
        });
    }
    
    std::vector<std::thread> workers;
    const int num_threads = 3;
    
    std::cout << "Starting " << num_threads << " worker threads" << std::endl;
    
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&io, i]() {
            std::cout << "Worker thread " << i << " (ID: " 
                      << std::this_thread::get_id() << ") starting" << std::endl;
            io.run();
            std::cout << "Worker thread " << i << " finished" << std::endl;
        });
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "Completed tasks: " << completed_tasks << std::endl;
}

void analyze_work_guard() {
    std::cout << "\n=== Analysis 4: work_guard Mechanism ===" << std::endl;
    
    // Without work_guard
    std::cout << "\n--- Without work_guard ---" << std::endl;
    asio::io_context io1;
    
    std::thread worker1([&io1]() {
        std::cout << "Worker thread starting io_context" << std::endl;
        size_t count = io1.run();
        std::cout << "Worker thread finished, executed " << count << " handlers" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Main thread: preparing to post task" << std::endl;
    
    asio::post(io1, []() {
        std::cout << "Delayed task executed" << std::endl;
    });
    
    worker1.join();
    
    // With work_guard
    std::cout << "\n--- With work_guard ---" << std::endl;
    asio::io_context io2;
    auto work_guard = asio::make_work_guard(io2);
    
    std::thread worker2([&io2]() {
        std::cout << "Worker thread starting io_context (with work_guard)" << std::endl;
        size_t count = io2.run();
        std::cout << "Worker thread finished, executed " << count << " handlers" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Main thread: posting delayed task" << std::endl;
    
    asio::post(io2, []() {
        std::cout << "Delayed task executed successfully" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Main thread: releasing work_guard" << std::endl;
    work_guard.reset();
    
    worker2.join();
}

void analyze_executor() {
    std::cout << "\n=== Analysis 5: Executor Concept ===" << std::endl;
    
    asio::io_context io;
    auto executor = io.get_executor();
    
    std::cout << "Using executor to post tasks:" << std::endl;
    
    asio::post(executor, []() {
        std::cout << "Task posted via executor" << std::endl;
    });
    
    asio::post(io, []() {
        std::cout << "Task posted directly via io_context" << std::endl;
    });
    
    io.run();
    
    std::cout << "Executor type: " << typeid(executor).name() << std::endl;
}

void analyze_exception_handling() {
    std::cout << "\n=== Analysis 6: Exception Handling ===" << std::endl;
    
    asio::io_context io;
    
    asio::post(io, []() {
        std::cout << "Task 1: Normal execution" << std::endl;
    });
    
    asio::post(io, []() {
        std::cout << "Task 2: About to throw exception" << std::endl;
        throw std::runtime_error("Test exception");
    });
    
    asio::post(io, []() {
        std::cout << "Task 3: Will this task execute?" << std::endl;
    });
    
    try {
        io.run();
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        std::cout << "io_context state - stopped(): " << io.stopped() << std::endl;
    }
}

int main() {
    std::cout << "Boost.Asio io_context Deep Analysis" << std::endl;
    std::cout << "===================================" << std::endl;
    
    analyze_basic_lifecycle();
    analyze_run_methods();
    analyze_multithreading();
    analyze_work_guard();
    analyze_executor();
    analyze_exception_handling();
    
    std::cout << "\nAnalysis completed!" << std::endl;
    return 0;
}