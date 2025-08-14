// Boost.Asio Learning Examples - Stage 5: Strand and Thread Safety
// Compile command: g++ -std=c++17 05_strand_thread_safety.cpp -lboost_system -pthread

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// Example 1: Basic Strand usage
class BasicStrandExample {
private:
    asio::io_context& io_;
    asio::strand<asio::io_context::executor_type> strand_;

public:
    BasicStrandExample(asio::io_context& io) : io_(io), strand_(asio::make_strand(io)) {}

    void start() {
        std::cout << "\n=== Basic Strand Example ===" << std::endl;
        
        // Submit tasks to the same strand from multiple threads
        for (int i = 0; i < 10; ++i) {
            asio::post(strand_, [this, i]() {
                std::cout << "Task " << i << " executed in thread " 
                          << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
            
            // Can also use dispatch
            asio::dispatch(strand_, [this, i]() {
                std::cout << "Dispatch task " << i << " executed in thread " 
                          << std::this_thread::get_id() << std::endl;
            });
        }
    }
};

// Example 2: Multi-Strand concurrency
class MultiStrandExample {
private:
    asio::io_context& io_;
    std::vector<std::unique_ptr<asio::strand<asio::io_context::executor_type>>> strands_;

public:
    MultiStrandExample(asio::io_context& io, int strand_count) : io_(io) {
        for (int i = 0; i < strand_count; ++i) {
            strands_.push_back(std::make_unique<asio::strand<asio::io_context::executor_type>>(asio::make_strand(io)));
        }
    }

    void start() {
        std::cout << "\n=== Multi-Strand Example ===" << std::endl;
        
        // Distribute tasks across different strands
        for (int i = 0; i < 15; ++i) {
            auto& strand = *strands_[i % strands_.size()];
            asio::post(strand, [this, i]() {
                std::cout << "Multi-strand task " << i 
                          << " [Thread ID: " << std::this_thread::get_id() << "]" << std::endl;
                
                // Simulate different processing times
                std::this_thread::sleep_for(std::chrono::milliseconds(50 + (i % 3) * 50));
            });
        }
    }
};

// Example 3: Strand wrapper functions
class StrandWrapperExample {
private:
    asio::io_context& io_;
    asio::strand<asio::io_context::executor_type> strand_;
    std::atomic<int> counter_{0};

public:
    StrandWrapperExample(asio::io_context& io) : io_(io), strand_(asio::make_strand(io)) {}

    void start() {
        std::cout << "\n=== Strand Wrapper Example ===" << std::endl;
        
        // Use bind_executor to create thread-safe callbacks
        auto safe_callback = asio::bind_executor(strand_, [this](const std::string& data) {
            counter_++;
            std::cout << "Safe callback: " << data << " (count: " << counter_ << ")" << std::endl;
        });
        
        // Call wrapped function from multiple threads
        for (int i = 0; i < 10; ++i) {
            std::thread([safe_callback, i]() {
                safe_callback("Data from thread " + std::to_string(i));
            }).detach();
        }
        
        // Periodic data printing
        auto timer = std::make_shared<asio::steady_timer>(io_, std::chrono::seconds(1));
        timer->async_wait(asio::bind_executor(strand_, [this, timer](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "Current counter value: " << counter_ << std::endl;
            }
        }));
    }
};

// Example 4: Strand usage in network connections
class NetworkStrandExample {
private:
    asio::io_context& io_;
    asio::strand<asio::io_context::executor_type> strand_;
    tcp::socket socket_;
    std::vector<std::string> message_queue_;

public:
    NetworkStrandExample(asio::io_context& io) 
        : io_(io), strand_(asio::make_strand(io)), socket_(io) {}

    void start() {
        std::cout << "\n=== Network Strand Example ===" << std::endl;
        
        // Simulate connection
        tcp::endpoint endpoint(tcp::v4(), 8080);
        socket_.async_connect(endpoint, 
            asio::bind_executor(strand_, [this](const boost::system::error_code& ec) {
                if (!ec) {
                    std::cout << "Connected successfully" << std::endl;
                    start_operations();
                }
            }));
    }

private:
    void send_message(const std::string& message) {
        // This function can be called from any thread, use post to ensure thread safety
        asio::post(strand_, [this, message]() {
            bool write_in_progress = !message_queue_.empty();
            message_queue_.push_back(message);
            if (!write_in_progress) {
                start_write();
            }
        });
    }

    void start_write() {
        auto buffer = asio::buffer(message_queue_.front());
        asio::async_write(socket_, buffer,
            asio::bind_executor(strand_, [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    message_queue_.erase(message_queue_.begin());
                    if (!message_queue_.empty()) {
                        start_write(); // Continue writing next message in queue
                    }
                }
            }));
    }

    void start_read() {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        socket_.async_read_some(asio::buffer(*buffer),
            asio::bind_executor(strand_, [this, buffer](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    // Process received data
                    start_read(); // Continue reading
                }
            }));
    }

    void start_operations() {
        start_read();
        // Send some test messages
        for (int i = 0; i < 5; ++i) {
            send_message("Test message " + std::to_string(i));
        }
    }
};

// Example 5: Comparison with/without Strand
class ComparisonExample {
private:
    asio::io_context& io_;
    asio::strand<asio::io_context::executor_type> strand_;
    std::vector<int> unsafe_data_;
    std::vector<int> safe_data_;
    std::mutex unsafe_mutex_; // For comparison with traditional mutex

public:
    ComparisonExample(asio::io_context& io) : io_(io), strand_(asio::make_strand(io)) {}

    void start() {
        std::cout << "\n=== Comparison Example ===" << std::endl;
        
        // Unsafe concurrent access (may cause data races)
        std::cout << "Starting unsafe concurrent access..." << std::endl;
        for (int i = 0; i < 100; ++i) {
            asio::post(io_, [this, i]() {
                unsafe_operation(i);
            });
        }
        
        // Safe access using traditional mutex
        std::cout << "Starting mutex-safe access..." << std::endl;
        for (int i = 0; i < 100; ++i) {
            asio::post(io_, [this, i]() {
                mutex_safe_operation(i);
            });
        }
        
        // Safe access using Strand
        std::cout << "Starting Strand-safe access..." << std::endl;
        for (int i = 0; i < 100; ++i) {
            asio::post(strand_, [this, i]() {
                strand_safe_operation(i);
            });
        }
        
        // Periodic result checking
        check_results();
    }
    
private:
    void unsafe_operation(int value) {
        // Data races may occur here
        unsafe_data_.push_back(value);
    }
    
    void mutex_safe_operation(int value) {
        std::lock_guard<std::mutex> lock(unsafe_mutex_);
        safe_data_.push_back(value);
    }
    
    void strand_safe_operation(int value) {
        // Strand ensures these operations execute serially, no explicit locking needed
        safe_data_.push_back(value + 1000); // Add 1000 to distinguish
    }
    
    void check_results() {
        auto timer = std::make_shared<asio::steady_timer>(io_, std::chrono::seconds(2));
        auto self = this;
        timer->async_wait([self, timer](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "\nResult statistics:" << std::endl;
                std::cout << "Unsafe data size: " << self->unsafe_data_.size() << std::endl;
                std::cout << "Safe data size: " << self->safe_data_.size() << std::endl;
                
                // Check data integrity
                bool unsafe_corrupted = self->unsafe_data_.size() != 100;
                bool safe_complete = self->safe_data_.size() == 200; // 100 + 100
                
                std::cout << "Unsafe data " << (unsafe_corrupted ? "may be" : "not") << " corrupted" << std::endl;
                std::cout << "Safe data " << (safe_complete ? "complete" : "incomplete") << std::endl;
            }
        });
    }
};

// Example 6: Custom Strand Manager
class StrandManager {
private:
    asio::io_context& io_;
    std::vector<std::unique_ptr<asio::strand<asio::io_context::executor_type>>> strands_;
    std::atomic<std::size_t> round_robin_counter_;
    
public:
    StrandManager(asio::io_context& io, std::size_t strand_count = 4) 
        : io_(io), round_robin_counter_(0) {
        
        for (std::size_t i = 0; i < strand_count; ++i) {
            strands_.push_back(std::make_unique<asio::strand<asio::io_context::executor_type>>(asio::make_strand(io)));
        }
        
        std::cout << "Created " << strand_count << " Strands" << std::endl;
    }
    
    // Get next available Strand (round-robin)
    asio::strand<asio::io_context::executor_type>& get_next_strand() {
        std::size_t index = round_robin_counter_++ % strands_.size();
        return *strands_[index];
    }
    
    // Get fixed Strand by key (ensures same key always uses same Strand)
    asio::strand<asio::io_context::executor_type>& get_strand_by_key(const std::string& key) {
        std::hash<std::string> hasher;
        std::size_t index = hasher(key) % strands_.size();
        return *strands_[index];
    }
    
    void demonstrate() {
        std::cout << "\n=== Strand Manager Example ===" << std::endl;
        
        // Distribute tasks using round-robin
        for (int i = 0; i < 12; ++i) {
            auto& strand = get_next_strand();
            asio::post(strand, [i]() {
                std::cout << "Round-robin task " << i 
                          << " [Thread ID: " << std::this_thread::get_id() << "]" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
        }
        
        // Distribute tasks by key (same user operations will execute in same Strand)
        std::vector<std::string> users = {"Alice", "Bob", "Charlie", "Alice", "Bob", "David"};
        for (const auto& user : users) {
            auto& strand = get_strand_by_key(user);
            asio::post(strand, [user]() {
                std::cout << "User " << user << " operation"
                          << " [Thread ID: " << std::this_thread::get_id() << "]" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            });
        }
    }
};

// Test function
void run_multi_threaded_test(std::function<void(asio::io_context&)> test_func, int thread_count = 4) {
    asio::io_context io;
    
    // Start test
    test_func(io);
    
    // Create multiple threads to run io_context
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&io]() {
            io.run();
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}

int main() {
    std::cout << "Boost.Asio Strand and Thread Safety Examples" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    try {
        std::cout << "\nSelect test mode:" << std::endl;
        std::cout << "1. Basic Strand usage" << std::endl;
        std::cout << "2. Multi-Strand concurrency" << std::endl;
        std::cout << "3. Strand wrapper functions" << std::endl;
        std::cout << "4. With/without Strand comparison" << std::endl;
        std::cout << "5. Strand manager" << std::endl;
        std::cout << "Please enter choice (1-5): ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                run_multi_threaded_test([](asio::io_context& io) {
                    BasicStrandExample example(io);
                    example.start();
                });
                break;
            }
            case 2: {
                run_multi_threaded_test([](asio::io_context& io) {
                    MultiStrandExample example(io, 3);
                    example.start();
                });
                break;
            }
            case 3: {
                run_multi_threaded_test([](asio::io_context& io) {
                    StrandWrapperExample example(io);
                    example.start();
                    
                    // Wait for some time to let all operations complete
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                });
                break;
            }
            case 4: {
                run_multi_threaded_test([](asio::io_context& io) {
                    ComparisonExample example(io);
                    example.start();
                });
                break;
            }
            case 5: {
                run_multi_threaded_test([](asio::io_context& io) {
                    StrandManager manager(io, 3);
                    manager.demonstrate();
                });
                break;
            }
            default:
                std::cout << "Invalid choice" << std::endl;
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Program exception: " << e.what() << std::endl;
    }
    
    std::cout << "\nStrand and thread safety examples completed!" << std::endl;
    return 0;
}