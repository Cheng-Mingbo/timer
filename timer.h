#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>

class Timer {
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using Task = std::function<void()>;
    
    Timer() : running_(false) {}
    
    ~Timer() {
        stop();
    }
    
    void start() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!running_.load()) {
            running_.store(true);
            worker_thread_ = std::jthread(&Timer::worker, this);
        }
    }
    
    void stop() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!running_.load()) {
                return;
            }
            running_.store(false);
            tasks_.clear();
            cv_.notify_all();
        }
        worker_thread_.request_stop();
    }
    
    void add_task(Duration delay, Task task) {
        std::unique_lock<std::mutex> lock(mutex_);
        TimePoint time = Clock::now() + delay;
        tasks_.emplace(time, std::move(task));
        cv_.notify_all();
    }
  
  private:
    void worker(std::stop_token stop_token) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!stop_token.stop_requested()) {
            if (tasks_.empty()) {
                cv_.wait(lock, [&stop_token]() { return stop_token.stop_requested(); });
            } else {
                auto now = Clock::now();
                auto it = tasks_.begin();
                if (it->first > now) {
                    cv_.wait_until(lock, it->first, [&stop_token]() { return stop_token.stop_requested(); });
                } else {
                    Task task = std::move(it->second);
                    tasks_.erase(it);
                    lock.unlock();
                    task();
                    lock.lock();
                }
            }
        }
    }
    
    std::map<TimePoint, Task> tasks_;
    std::mutex mutex_;
    std::condition_variable_any cv_;
    std::jthread worker_thread_;
    std::atomic<bool> running_;
};
