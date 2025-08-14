// Boost.Asio Learning Examples - Stage 1: Basic Concepts
// Compile command: g++ -std=c++17 01_basic_concepts.cpp -lboost_system -pthread

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>

namespace asio = boost::asio;

// Example 1: Basic io_context usage
void example1_basic_io_context() {
    std::cout << "\n=== Example 1: Basic io_context usage ===" << std::endl;
    
    asio::io_context io;
    
    // Post a simple task
    asio::post(io, []() {
        std::cout << "Task 1: Executing in io_context" << std::endl;
    });
    
    // Post another task
    asio::post(io, []() {
        std::cout << "Task 2: Also executing in io_context" << std::endl;
    });
    
    std::cout << "Starting io_context..." << std::endl;
    io.run();  // Run the event loop
    std::cout << "io_context completed" << std::endl;
}

// Example 2: Multithreaded io_context
void example2_multithreaded_io_context() {
    std::cout << "\n=== Example 2: Multithreaded io_context ===" << std::endl;
    
    asio::io_context io;
    
    // Post multiple tasks
    for (int i = 0; i < 5; ++i) {
        asio::post(io, [i]() {
            std::cout << "Task " << i << " executing in thread 22 " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Create multiple threads to run io_context
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&io]() {
            io.run();
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
}

// Example 3: work_guard usage
void example3_work_guard() {
    std::cout << "\n=== Example 3: work_guard usage ===" << std::endl;
    
    asio::io_context io;
    
    // Create work_guard to prevent io_context from exiting early
    auto work_guard = asio::make_work_guard(io);
    
    std::thread worker([&io]() {
        std::cout << "Worker thread starting io_context" << std::endl;
        io.run();
        std::cout << "Worker thread finished" << std::endl;
    });
    
    // Post some tasks
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        asio::post(io, [i]() {
            std::cout << "Delayed task " << i << " executing" << std::endl;
        });
    }
    
    std::cout << "Releasing work_guard" << std::endl;
    work_guard.reset();  // Release work_guard, allow io_context to exit
    
    worker.join();
}

// Example 4: dispatch vs post
void example4_dispatch_vs_post() {
    std::cout << "\n=== Example 4: dispatch vs post difference ===" << std::endl;
    
    asio::io_context io;
    
    asio::post(io, [&io]() {
        std::cout << "In io_context thread..." << std::endl;
        
        // dispatch: If current thread is io_context thread, execute immediately
        asio::dispatch(io, []() {
            std::cout << "dispatch: Execute immediately (current thread)" << std::endl;
        });
        
        // post: Always put in queue, execute later
        asio::post(io, []() {
            std::cout << "post: Put in queue, execute later" << std::endl;
        });
        
        std::cout << "Continuing current task..." << std::endl;
    });
    
    io.run();
}

// Example 5: Error handling
void example5_error_handling() {
    std::cout << "\n=== Example 5: Error handling ===" << std::endl;
    
    asio::io_context io;
    
    // Use error_code for error handling
    asio::post(io, []() {
        boost::system::error_code ec;
        
        // Simulate an operation that might fail
        // Here we manually set an error
        ec = boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
        
        if (ec) {
            std::cout << "Error occurred: " << ec.message() << std::endl;
        } else {
            std::cout << "Operation successful" << std::endl;
        }
    });
    
    // Exception handling example
    asio::post(io, []() {
        try {
            // Simulate operation that might throw exception
            throw std::runtime_error("Simulated exception");
        } catch (const std::exception& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }
    });
    
    io.run();
}

int main() {
    std::cout << "Boost.Asio Basic Concepts Learning Examples" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    try {
        example1_basic_io_context();
        example2_multithreaded_io_context();
        example3_work_guard();
        example4_dispatch_vs_post();
        example5_error_handling();
    } catch (const std::exception& e) {
        std::cerr << "Program exception: " << e.what() << std::endl;
    }
    
    std::cout << "\nAll examples completed!" << std::endl;
    return 0;
}