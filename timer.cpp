#include "timer.h"

Timer::Timer() : running(false) {
}

void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
    running = true;
}

void Timer::stop() {
    if (running) {
        end_time = std::chrono::high_resolution_clock::now();
        running = false;
    }
}

double Timer::getElapsedMilliseconds() const {
    if (running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return duration.count() / 1000.0;
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
}

double Timer::getElapsedMicroseconds() const {
    if (running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return static_cast<double>(duration.count());
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return static_cast<double>(duration.count());
    }
}

double Timer::getElapsedSeconds() const {
    return getElapsedMilliseconds() / 1000.0;
}

void Timer::printElapsed(const char* label) const {
    double elapsed = getElapsedMilliseconds();
    std::cout << label << ": " << elapsed << " ms" << std::endl;
}

void Timer::reset() {
    running = false;
    start_time = std::chrono::high_resolution_clock::time_point();
    end_time = std::chrono::high_resolution_clock::time_point();
}

bool Timer::isRunning() const {
    return running;
}

double Timer::measureFunction(void (*func)()) {
    Timer timer;
    timer.start();
    func();
    timer.stop();
    return timer.getElapsedMilliseconds();
}