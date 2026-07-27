#include "cpuBoundProcessExecution.hpp"
#include "../utils/logger.hpp"

// Standard Linux value for nice 0
constexpr int NICE_0_LOAD = 1024;

// Higher priority → higher weight → slower vruntime growth

double weightFunction(int priority) {
    // +1 to avoid division by zero

    return NICE_0_LOAD / (double)(priority + 1);
}

// Execute CPU-bound process for one time slice

void executeCpuBoundProcess(Process* process, int timeSlice, QueueService& queue) {
    

    // Step 1: Determine how much to execute
    // We can't execute more than the remaining burst time
    const int executedTime = std::min(timeSlice, process->cpu_burst_time);
    
    // Step 2: Reduce remaining CPU time
    process->cpu_burst_time -= executedTime;
    
    
    // Step 3: Calculate weight based on priority
    const double weight = weightFunction(process->priority);
    

    // Step 4: Update vruntime
    // Formula: vruntime += (executed_time * NICE_0_LOAD) / weight
    // 
    // Key Insight: Higher weight → smaller vruntime increase
    // This means higher priority processes get more CPU time
    // because their vruntime grows more slowly
    process->vruntime += (long long)((executedTime * NICE_0_LOAD) / weight);
    

    // Step 5: Update statistics
    process->totalCpuTime += executedTime;
    process->contextSwitches++;
    

    // Step 6: Log execution for debugging
    Logger::debug("Process " + std::to_string(process->pid) + 
                  " executed for " + std::to_string(executedTime) + 
                  "ms, vruntime: " + std::to_string(process->vruntime));
    

    // Step 7: Requeue if more work remains
    if (process->cpu_burst_time > 0) {
        queue.push(process);  // Still has work to do
    } else {

        // Process has completed all its work
        process->state.state = COMPLETED;
        process->deleted = true;
        Logger::info("Process " + std::to_string(process->pid) + " completed");
    }
}