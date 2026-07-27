#include "ioBoundProcessExecution.hpp"
#include "../utils/logger.hpp"
#include <thread>
#include <chrono>

// Standard Linux value for nice 0
constexpr int NICE_0_LOAD = 1024;

// Handle I/O-bound process execution

void handleIoBoundProcess(Process* process, int ioWaitTime, QueueService& queue) {
    
    // Step 1: Simulate I/O wait
    // The process is blocked waiting for I/O (file, network, etc.)
    Logger::debug("Process " + std::to_string(process->pid) + 
                  " waiting for I/O (" + std::to_string(ioWaitTime) + "ms)");
    
    // Store start time for accurate duration measurement
    auto ioStartTime = std::chrono::steady_clock::now();
    process->state.state = BLOCKED_IO;  // Update process state
    
    
    // Sleep to simulate I/O wait
    // This is where the process would be waiting for hardware
    std::this_thread::sleep_for(std::chrono::milliseconds(ioWaitTime));
    
    
    // Update statistics
    process->totalIoTime += ioWaitTime;
    process->state.state = READY;  // Back to ready state
    
    
    // Calculate actual I/O duration (in case of system delays)
    auto ioEndTime = std::chrono::steady_clock::now();
    auto ioDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        ioEndTime - ioStartTime).count();
    
    
        // Step 2: Penalize vruntime for I/O wait time
    // Even though process wasn't using CPU, its vruntime increases
    // This prevents I/O-bound processes from dominating the CPU
    const double weight = weightFunction(process->priority);
    process->vruntime += (long long)((ioDuration * NICE_0_LOAD) / weight);
    
    Logger::debug("Process " + std::to_string(process->pid) + 
                  " I/O completed, penalty applied. vruntime: " + 
                  std::to_string(process->vruntime));
    
    
    
    // Step 3: Execute CPU portion (1 time slice)
    // After I/O completes, process gets a small CPU time
    const int executedTime = 1;  // Same as CPU time slice
   
    
    // Step 4: Update vruntime for CPU portion
    process->cpu_burst_time -= executedTime;
    process->vruntime += (long long)((executedTime * NICE_0_LOAD) / weight);
    

    // Step 5: Update statistics
    process->totalCpuTime += executedTime;
    process->contextSwitches++;
    

    // Step 6: Requeue if more work remains
    if (process->cpu_burst_time > 0) {
        queue.push(process);  // Still has work to do
    }
    else {
        // Process has completed all its work
        process->state.state = COMPLETED;
        Logger::info("Process " + std::to_string(process->pid) + " completed");
    }
}