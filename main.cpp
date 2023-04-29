#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <thread>
#include "timer.h"

void precision_test() {
    Timer timer;
    timer.start();
    
    const int num_tasks = 5;
    std::vector<std::chrono::milliseconds> expected_delays = {
            std::chrono::milliseconds(100),
            std::chrono::milliseconds(200),
            std::chrono::milliseconds(300),
            std::chrono::milliseconds(400),
            std::chrono::milliseconds(500),
    };
    
    std::vector<Timer::TimePoint> actual_times(num_tasks);
    
    Timer::TimePoint start_time = Timer::Clock::now();
    for (int i = 0; i < num_tasks; ++i) {
        timer.add_task(std::chrono::duration_cast<Timer::Duration>(expected_delays[i]), [&, i]() {
            actual_times[i] = Timer::Clock::now();
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << std::setw(10) << "Task"
              << std::setw(15) << "Expected"
              << std::setw(15) << "Actual"
              << std::setw(15) << "Difference" << std::endl;
    
    for (int i = 0; i < num_tasks; ++i) {
        Timer::Duration expected_time = std::chrono::duration_cast<Timer::Duration>(expected_delays[i]);
        Timer::Duration actual_time = actual_times[i] - start_time;
        Timer::Duration difference = actual_time - expected_time;
        
        std::cout << std::setw(10) << i
                  << std::setw(15) << expected_time.count()
                  << std::setw(15) << actual_time.count()
                  << std::setw(15) << difference.count() << std::endl;
    }
}


int main() {
    precision_test();
    return 0;
}
