#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <vector>      
#include <map>         // For key-value data
#include <atomic>      // For thread-safe flags
#include <mutex>       // For thread safety
#include <thread>      
#include <functional>  // For callbacks

#include "../core/processService.hpp"
#include "../config/config.hpp"  // For MonitorConfig

// Forward declaration to avoid circular include
class cfs;

/* MonitorData - Data structure for monitoring updates
 * 
 * Contains all information needed for real-time visualization:
 * - Current state
 * - Queue information
 * - Statistics
 * - Per-process data
 */
struct MonitorData {
    long long timestamp;                          // When data was collected
    int currentProcess;                           // Currently running process
    size_t queueSize;                             // Number of processes waiting
    std::vector<Process*> processes;              // All processes in queue
    long long contextSwitches;                    // Total context switches
    long long totalCpuTime;                       // Total CPU time used
    std::map<int, long long> processCpuTime;      // CPU time per process
};

/* Monitor - Real-time monitoring system
 * 
 * Features:
 * - Collects data from scheduler periodically
 * - Broadcasts to registered callbacks
 * - Maintains history for analysis
 * - Thread-safe implementation
 */
class Monitor {
private:
    MonitorConfig config;                         // Configuration
    std::atomic<bool> running{false};             // Running flag
    std::thread monitorThread;                    // Background thread
    mutable std::mutex dataMutex;                 // Mutex for data protection
    MonitorData currentData;                      // Current data
    std::vector<MonitorData> history;             // Historical data
    std::vector<std::function<void(const MonitorData&)>> callbacks; // Callbacks

    void broadcastData(const MonitorData& data);

public:
    Monitor(const MonitorConfig& cfg = MonitorConfig());
    ~Monitor();

    // Changed from Scheduler& to cfs&
    void start(cfs& scheduler);
    void stop();

    void update(const MonitorData& data);
    void registerCallback(std::function<void(const MonitorData&)> callback);

    MonitorData getCurrentData() const;
    std::vector<MonitorData> getHistory() const;
    void saveToFile(const std::string& filename);
};

#endif // MONITOR_HPP