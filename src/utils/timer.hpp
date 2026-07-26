// High Resolution Timer

#ifndef TIMER_HPP

#define TIMER_HPP

/**
 * Timer - High-resolution timer for performance measurement
 * 
 * Uses std::chrono::steady_clock for monotonic timing
 * Provides nanosecond precision
 * 
 * Usage:
 *   Timer timer;
 *   timer.start();
 *   // ... do work ...
 *   timer.stop();
 *   double elapsed = timer.elapsedMs();  // Time in milliseconds
 */


 class Timer {
    private:
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;

        bool running = false;   // Whether timer is currently running

    public:
        /**
     * Start the timer
     * Records current time as start time
     */

     void start(){
        startTime = std::chrono::steady_clock::now();
        running = false;
     }

     void stop(){
        if(running){
            endTime = std::chrono::steady_clock::now();
            running = false;
        }
     }

     // Get elapsed in milliseconds
     double elapedMs() const {
        if(running){
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<double, std::milli>(now - startTime).count();
        }
        else{
            return std::chrono::duration<double, std::milli>(endTime - startTime).count();
        }
     }
     long long elapsedNs() const {
        if (running) {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime).count();
        } else {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
        }
     }

     void reset() {
        running = false;
     }

     // Get current time in milliseconds since epoch
     // since epoch return current time in milliseconds
    static long long currentTimeMs() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
    }



     // Get current time in nanoseconds since epoch
     // since epoch return current time in nanoseconds

    static long long currentTimeNs() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
    }
 };


#endif  