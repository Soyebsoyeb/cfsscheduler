#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <vector>      
#include <map>         // For key-value data
#include <atomic>      // For thread-safe flags
#include <mutex>       // For thread safety
#include <thread>      
#include <functional>  // For callbacks

#include "../core/processService.hpp"

/* MonitorData - Data structure for monitoring updates
 
 *  Contains all information needed for real-time visualization:
  - Current state
  - Queue information
  - Statistics
  - Per-process data
 */

 struct MonitorData {
    long long timestamp;                          // When data was collected
    int currentProcess;                           
    size_t queueSize;                             // Number of processes waiting
    std::vector<Process*> processes;              // All processes in queue
    long long contextSwitches;                    // Total context switches
    long long totalCpuTime;                       // Total CPU time used
    std::map<int, long long> processCpuTime;      // CPU time per process
};


//  MonitorConfig - Configuration for the monitor

struct MonitorConfig {
    bool enabled = true;          // Enable/disable monitoring
    int updateIntervalMs = 100;   // Update interval in milliseconds
    int port = 8080;              // Port for web dashboard
    int maxHistory = 1000;        // Maximum historical data points
    bool websocketEnabled = true; // Enable WebSocket support
};

/*  Monitor - Real-time monitoring system
  

 * Features:
   - Collects data from scheduler periodically
   - Broadcasts to registered callbacks
   - Maintains history for analysis
   - Thread-safe implementation
   
*/


class Monitor {
private:
    MonitorConfig config;                        // Configuration
    std::atomic<bool> running{false};            // Running flag
    std::thread monitorThread;                   // Background thread
    mutable std::mutex dataMutex;                // Mutex for data protection
    MonitorData currentData;                     // Current data
    std::vector<MonitorData> history;            // Historical data
    
    // Callbacks for data updates
    std::vector<std::function<void(const MonitorData&)>> callbacks;
    

    void broadcastData(const MonitorData& data);

public:


    Monitor(const MonitorConfig& cfg = MonitorConfig());
    
    ~Monitor();
    
    void start(class Scheduler& scheduler);

    void stop();
    

    void update(const MonitorData& data);
    
    void registerCallback(std::function<void(const MonitorData&)> callback);
    
    MonitorData getCurrentData() const;
    

    std::vector<MonitorData> getHistory() const;
    
    
    void saveToFile(const std::string& filename);
};

#endif