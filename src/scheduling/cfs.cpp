#include "cfs.hpp"
#include "../utils/logger.hpp"
#include "../utils/timer.hpp"
#include <chrono>
#include <thread>

cfs::cfs(const Config& cfg) : config(cfg) {
    stats.startTime = getCurrentTimeMs();
    Logger::info("CFS Scheduler initialized");
}

cfs::~cfs() {
    // IMPORTANT: Processes are already deleted when they complete.
    // The queue should be empty, but just in case, clear it without deleting.
    while (!queue.empty()) {
        queue.pop();  // Just pop, don't delete
    }
    
    // Clean up execution logs
    for (auto log : logs) {
        delete log;
    }
    
    Logger::info("CFS Scheduler destroyed");
}

// Main scheduling loop
std::vector<ProcessLog*> cfs::schedule(
    std::vector<Process*> processList, 
    Monitor* monitor) {
    
    // Store monitor reference for updates
    activeMonitor = monitor;
    running = true;
  
    // Step 1: Add all processes to the runqueue
    for (auto process : processList) {
        process->arrivalTime = getCurrentTimeMs();
        process->state.state = READY;
        process->deleted = false;
        queue.push(process);
    }
    
    Logger::info("Starting CFS scheduling with " + 
                 std::to_string(processList.size()) + " processes");
    
    // Step 2: Main scheduling loop
    while (!queue.empty() && running) {
        // Step 2a: Pick process with smallest vruntime
        currentProcess = queue.top();
        queue.pop();
        
        // Update process state
        currentProcess->state.state = RUNNING;
        currentProcess->state.lastStateChange = getCurrentTimeMs();
        currentProcess->lastScheduled = getCurrentTimeMs();
        
        // Record start time
        long long startTimeMs = getCurrentTimeMs();
        
        // Step 2b: Execute based on process type
        if (currentProcess->processNature == PROCESS_NATURE::CPU_BOUND) {
            executeCpuBoundProcess(currentProcess, config.scheduler.timeSliceMs, queue);
        } else {
            handleIoBoundProcess(currentProcess, config.scheduler.ioWaitMs, queue);
        }
        
        // Record end time
        long long endTimeMs = getCurrentTimeMs();
        long long cpuTime = endTimeMs - startTimeMs;
        
        // Step 2c: Update statistics
        stats.contextSwitches++;
        stats.totalCpuTime += cpuTime;
        stats.processCpuTime[currentProcess->pid] += cpuTime;
        
        // Step 2d: Log execution
        createProcessLog(startTimeMs, endTimeMs, currentProcess->pid);
        
        // Step 2e: Send monitoring update (if configured)
        if (activeMonitor && (endTimeMs - lastUpdateTime > 
                              config.monitoring.updateIntervalMs)) {
            sendMonitoringUpdate();
            lastUpdateTime = endTimeMs;
        }
        
        // Step 2f: Update process state
        if (currentProcess->state.state != COMPLETED) {
            currentProcess->state.state = READY;
        }
        currentProcess->state.lastStateChange = getCurrentTimeMs();
    }
    
    Logger::info("Scheduling complete. " + 
                 std::to_string(logs.size()) + " execution events logged");
    
    return logs;
}

// Create a process execution log
void cfs::createProcessLog(long long startTime, long long endTime, int pid) {
    ProcessLog* log = new ProcessLog();
    log->pid = pid;
    log->startTime = startTime;
    log->endTime = endTime;
    logs.push_back(log);
}

// Send monitoring update to monitor system
void cfs::sendMonitoringUpdate() {
    if (!activeMonitor) return;
    
    MonitorData data;
    data.timestamp = getCurrentTimeMs();
    data.currentProcess = currentProcess ? currentProcess->pid : -1;
    data.queueSize = queue.size();
    data.contextSwitches = stats.contextSwitches.load();
    data.totalCpuTime = stats.totalCpuTime.load();
    
    std::vector<Process*> tempQueue;
    while (!queue.empty()) {
        Process* p = queue.top();
        queue.pop();
        data.processes.push_back(p);
        tempQueue.push_back(p);
    }
    
    for (auto p : tempQueue) {
        queue.push(p);
    }
    
    for (const auto& pair : stats.processCpuTime) {
        data.processCpuTime[pair.first] = pair.second;
    }
    
    activeMonitor->update(data);
}

// Print scheduler statistics to console
void cfs::printStatistics() const {
    std::cout << "\n  CFS Scheduler Statistics " << std::endl;
    std::cout << "Total Context Switches: " << stats.contextSwitches.load() << std::endl;
    std::cout << "Total CPU Time: " << stats.totalCpuTime.load() << "ms" << std::endl;
    std::cout << "Total I/O Time: " << stats.totalIoTime.load() << "ms" << std::endl;
    std::cout << "Execution Logs: " << logs.size() << std::endl;
    
    if (!stats.processCpuTime.empty()) {
        std::cout << "\nPer-Process Statistics:" << std::endl;
        for (const auto& pair : stats.processCpuTime) {
            std::cout << "  Process " << pair.first << ": " << pair.second << "ms CPU" << std::endl;
        }
    }
}

long long cfs::getCurrentTimeMs() {
    return Timer::currentTimeMs();
}