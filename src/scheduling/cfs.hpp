#ifndef CFS_HPP
#define CFS_HPP

#include <vector>     
#include <atomic>      
#include <map>         // For statistics storage

#include "../core/processService.hpp"  // Process definition
#include "../core/processLog.hpp"      // Log definition
#include "queueService.hpp"            // Runqueue
#include "cpuBoundProcessExecution.hpp" // CPU-bound execution
#include "ioBoundProcessExecution.hpp"  // I/O-bound execution
#include "../config/config.hpp"         // Configuration
#include "../monitoring/monitor.hpp"    // Monitoring

/*  CFS (Completely Fair Scheduler) Implementation
  
  This implements the core CFS scheduling algorithm from the Linux kernel
  
 * Core concepts:
  1. Each process has a vruntime (virtual runtime)
  2. The process with the smallest vruntime runs next
  3. Higher priority = higher weight = slower vruntime growth = more CPU time
  4. I/O-bound processes are penalized for waiting
  
  The scheduler maintains fairness while respecting priorities

 */

 
class cfs {
private:
    Config config;                      // Configuration settings
    QueueService queue;                 // Runqueue (min-heap by vruntime)
    std::vector<ProcessLog*> logs;      // Execution logs


    struct Statistics {
        std::atomic<long long> contextSwitches{0};  // Total context switches
        std::atomic<long long> totalCpuTime{0};      // Total CPU time used
        std::atomic<long long> totalIoTime{0};       // Total I/O time
        std::atomic<long long> totalWaitTime{0};     // Total waiting time
        long long startTime;                         // Scheduler start time
        std::map<int, long long> processCpuTime;     // CPU time per process
        std::map<int, long long> processWaitTime;    // Wait time per process
        std::map<int, long long> processIoTime;      // I/O time per process
    } stats;
    
    Process* currentProcess = nullptr;   // Currently running process
    std::atomic<bool> running{true};    // Running flag
    
    Monitor* activeMonitor = nullptr;    // Monitoring system
    long long lastUpdateTime = 0;        // Last monitoring update time
    

    // Create ProcessLog
    void createProcessLog(long long startTime, long long endTime, int pid);
    
    
    // Send monitoring update to monitor system Called periodically during scheduling
    
    void sendMonitoringUpdate();

    long long getCurrentTimeMs();


public:


    cfs(const Config& cfg = Config());

    ~cfs();
    
    /* Main scheduling function
      
     * This is the heart of the scheduler
      
     * Algorithm:
      1. Add all processes to the runqueue
      2. While processes remain:
         a. Pick process with smallest vruntime
         b. Execute based on process type (CPU or I/O bound)
         c. Update vruntime
         d. Requeue if not complete
         e. Log execution
         f. Update monitoring
      3. Return execution logs
   
     */


    std::vector<ProcessLog*> schedule(
        std::vector<Process*> processList, 
        Monitor* monitor = nullptr);
    

    void printStatistics() const;
    

    Process* getCurrentProcess() const { return currentProcess; }
    
    
    void stop() { running = false; }
    

    void setMonitor(Monitor* monitor) { activeMonitor = monitor; }
    
    
    const Statistics& getStatistics() const { return stats; }
};

#endif