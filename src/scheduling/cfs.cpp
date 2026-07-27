#include "cfs.hpp"
#include "../utils/logger.hpp"
#include "../utils/timer.hpp"
#include <chrono>
#include <thread>
#include <fstream>

cfs::cfs(const Config& cfg) : config(cfg) {
    stats.startTime = getCurrentTimeMs();
    Logger::info("CFS Scheduler initialized");
}

cfs::~cfs() {
    while (!queue.empty()) {
        queue.pop();
    }
    
    Logger::info("CFS Scheduler destroyed");
}

std::vector<ProcessLog*> cfs::schedule(
    std::vector<Process*> processList, 
    Monitor* monitor) {
    
    activeMonitor = monitor;
    running = true;
  
    std::ofstream detailLog("execution_details.csv");
    detailLog << "Event,Process,StartTime,EndTime,Duration,VRuntime,Priority,Remaining,State,Type\n";
    
    Logger::info("--- Loading Processes ---");
    for (auto process : processList) {
        process->arrivalTime = getCurrentTimeMs();
        process->state.state = READY;
        queue.push(process);
        
        std::string nature = (process->processNature == PROCESS_NATURE::CPU_BOUND) ? "CPU" : "IO";
        Logger::info("Loaded Process " + std::to_string(process->pid) + 
                     " | vruntime=" + std::to_string(process->vruntime) +
                     " | priority=" + std::to_string(process->priority) +
                     " | burst=" + std::to_string(process->cpu_burst_time) + "ms" +
                     " | nature=" + nature);
    }
    
    Logger::info("Starting CFS scheduling with " + 
                 std::to_string(processList.size()) + " processes");
    Logger::info("--------------------------------------------------");
    
    int eventCount = 0;
    
    while (!queue.empty() && running) {
        eventCount++;
        
        currentProcess = queue.top();
        queue.pop();
        
        std::string nature = (currentProcess->processNature == PROCESS_NATURE::CPU_BOUND) ? "CPU" : "IO";
        Logger::info("--- Event #" + std::to_string(eventCount) + " ---");
        Logger::info("Selected Process " + std::to_string(currentProcess->pid) + 
                     " | vruntime=" + std::to_string(currentProcess->vruntime) +
                     " | priority=" + std::to_string(currentProcess->priority) +
                     " | remaining=" + std::to_string(currentProcess->cpu_burst_time) + "ms" +
                     " | type=" + nature);
        
        currentProcess->state.state = RUNNING;
        currentProcess->state.lastStateChange = getCurrentTimeMs();
        currentProcess->lastScheduled = getCurrentTimeMs();
        
        long long startTimeMs = getCurrentTimeMs();
        
        if (currentProcess->processNature == PROCESS_NATURE::CPU_BOUND) {
            Logger::debug("  Executing CPU-BOUND");
            executeCpuBoundProcess(currentProcess, config.scheduler.timeSliceMs, queue);
        } else {
            Logger::debug("  Executing IO-BOUND (waiting " + 
                          std::to_string(config.scheduler.ioWaitMs) + "ms)");
            handleIoBoundProcess(currentProcess, config.scheduler.ioWaitMs, queue);
        }
        
        long long endTimeMs = getCurrentTimeMs();
        long long cpuTime = endTimeMs - startTimeMs;
        
        Logger::info("  Completed in " + std::to_string(cpuTime) + "ms" +
                     " | new vruntime=" + std::to_string(currentProcess->vruntime) +
                     " | remaining=" + std::to_string(currentProcess->cpu_burst_time) + "ms");
        
        if (currentProcess->state.state != COMPLETED) {
            stats.contextSwitches++;
            stats.totalCpuTime += cpuTime;
            stats.processCpuTime[currentProcess->pid] += cpuTime;
        }
        
        createProcessLog(startTimeMs, endTimeMs, currentProcess->pid);
        
        detailLog << eventCount << ","
                  << currentProcess->pid << ","
                  << startTimeMs << ","
                  << endTimeMs << ","
                  << cpuTime << ","
                  << currentProcess->vruntime << ","
                  << currentProcess->priority << ","
                  << currentProcess->cpu_burst_time << ","
                  << (currentProcess->state.state == COMPLETED ? "COMPLETED" : "READY") << ","
                  << nature << "\n";
        
        if (activeMonitor && (endTimeMs - lastUpdateTime > 
                              config.monitoring.updateIntervalMs)) {
            sendMonitoringUpdate();
            lastUpdateTime = endTimeMs;
        }
        
        if (currentProcess->state.state != COMPLETED) {
            currentProcess->state.state = READY;
            Logger::debug("  Process " + std::to_string(currentProcess->pid) + " requeued");
        } else {
            // Filter garbage PIDs in completion log
            if (currentProcess->pid > 0 && currentProcess->pid < 10000) {
                Logger::info("  Process " + std::to_string(currentProcess->pid) + " COMPLETED");
            } else {
                Logger::info("  Process COMPLETED (invalid PID)");
            }
        }
        currentProcess->state.lastStateChange = getCurrentTimeMs();
        
        if (!queue.empty()) {
            Process* next = queue.top();
            Logger::debug("  Next in queue: Process " + std::to_string(next->pid) + 
                          " (vruntime=" + std::to_string(next->vruntime) + ")");
        } else {
            Logger::debug("  Queue is empty");
        }
    }
    
    detailLog.close();
    Logger::info("--------------------------------------------------");
    Logger::info("Detailed execution log saved to execution_details.csv");
    
    if (activeMonitor) {
        MonitorData emptyData;
        emptyData.timestamp = getCurrentTimeMs();
        emptyData.currentProcess = -1;
        emptyData.queueSize = 0;
        emptyData.contextSwitches = stats.contextSwitches.load();
        emptyData.totalCpuTime = stats.totalCpuTime.load();
        activeMonitor->update(emptyData);
    }
    
    Logger::info("Scheduling complete. " + 
                 std::to_string(logs.size()) + " execution events logged");
    
    Logger::info("--- Execution Summary ---");
    std::map<int, int> execCount;
    for (auto log : logs) {
        if (log->pid > 0 && log->pid < 10000) {
            execCount[log->pid]++;
        }
    }
    for (const auto& pair : execCount) {
        Logger::info("Process " + std::to_string(pair.first) + 
                     " executed " + std::to_string(pair.second) + " times");
    }
    
    return logs;
}

void cfs::createProcessLog(long long startTime, long long endTime, int pid) {
    ProcessLog* log = new ProcessLog();
    log->pid = pid;
    log->startTime = startTime;
    log->endTime = endTime;
    logs.push_back(log);
}

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
        if (p && p->state.state != COMPLETED && p->pid > 0 && p->pid < 10000) {
            data.processes.push_back(p);
        }
        tempQueue.push_back(p);
    }
    
    for (auto p : tempQueue) {
        queue.push(p);
    }
    
    for (const auto& pair : stats.processCpuTime) {
        if (pair.first > 0 && pair.first < 10000) {
            data.processCpuTime[pair.first] = pair.second;
        }
    }
    
    activeMonitor->update(data);
}

void cfs::printStatistics() const {
    std::cout << "\n  CFS Scheduler Statistics " << std::endl;
    std::cout << "Total Context Switches: " << stats.contextSwitches.load() << std::endl;
    std::cout << "Total CPU Time: " << stats.totalCpuTime.load() << "ms" << std::endl;
    std::cout << "Total I/O Time: " << stats.totalIoTime.load() << "ms" << std::endl;
    std::cout << "Execution Logs: " << logs.size() << std::endl;
    
    if (!stats.processCpuTime.empty()) {
        std::cout << "\nPer-Process Statistics:" << std::endl;
        for (const auto& pair : stats.processCpuTime) {
            if (pair.first > 0 && pair.first < 10000) {
                std::cout << "  Process " << pair.first << ": " << pair.second << "ms CPU" << std::endl;
            }
        }
    }
}

long long cfs::getCurrentTimeMs() {
    return Timer::currentTimeMs();
}