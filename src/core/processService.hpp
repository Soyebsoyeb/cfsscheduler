#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>


#include "../../include/nlohmann/json.hpp"    // JSON Library
#include "../utils/logger.hpp"                // Logging


using json = nlohmann::json;

/**
 * PROCESS_NATURE 
  
 * CPU_BOUND: Process uses CPU continuously
    - Example: Mathematical computations, data processing
    - Scheduler: Executes for full time slice
    - vruntime: Increases proportionally to CPU time
  
 * IO_BOUND: Process frequently waits for I/O
    - Example: File operations, network requests, user input
    - Scheduler: Simulates I/O wait, then executes
    - vruntime: Penalized for I/O wait time
 */


 enum PROCESS_NATURE {
    CPU_BOUND,
    IO_BOUND
 };



/**
 * ProcessState - Current state of a process
 * 
 * READY: Waiting in the runqueue to be scheduled
 * RUNNING: Currently executing on the CPU
 * BLOCKED_IO: Waiting for I/O operation to complete
 * COMPLETED: Finished execution, no more work to do
 * WAITING: Waiting for some event (generic)
 */

 enum ProcessState {
    READY,
    RUNNING,
    BLOCKED_IO,
    COMPLETED,
    WAITING
 };


 /**
 * ProcessStateData - Additional state information
 
  counter: Generic counter for tracking progress
  state: Current state of the process
  lastStateChange: Timestamp of last state transition
 */


 struct ProcessStateData {
    long long counter = 0;               // Generic counter
    ProcessState state = READY;          // Current State
    long long lastStateChange = 0;
 }; 


 /**
 * Process - Main process structure
 * 
 * This is the core data structure representing a schedulable task
 * 
 * Fields:
    pid: Unique process identifier
    vruntime: Virtual runtime (CFS key metric)
    cpu_burst_time: Remaining CPU time needed (ms)
    io_burst_time: Remaining I/O time (ms)
    priority: Scheduling priority (1 = highest, higher number = lower priority)
    state: Current process state
    processNature: CPU or I/O bound
  
 * Statistics fields (for monitoring):
    totalCpuTime: Total CPU time used so far
    totalWaitTime: Total time waiting in queue
    totalIoTime: Total time waiting for I/O
    contextSwitches: Number of times scheduled
    lastScheduled: When last scheduled
    arrivalTime: When process was created
 */

struct Process {
    // Core scheduling fields
    int pid;                      // Process ID
    long long int vruntime;       // Virtual runtime (CFS metric)
    int cpu_burst_time;           // Remaining CPU time (ms)
    int io_burst_time;            // Remaining I/O time (ms)
    int priority;                 // Priority (1 = highest)
    ProcessStateData state;       // Current state
    PROCESS_NATURE processNature; // CPU or I/O bound
   
    
    // Statistics for monitoring
    long long totalCpuTime = 0;       // Total CPU time used
    long long totalWaitTime = 0;      // Total waiting time
    long long totalIoTime = 0;        // Total I/O time
    int contextSwitches = 0;          // Number of times scheduled
    long long lastScheduled = 0;      // Last scheduling time
    long long arrivalTime = 0;        // Creation time
};


/**
 * Convert string to PROCESS_NATURE enum
  
   "CPU_BOUND" or "IO_BOUND"
   Corresponding enum value
   std::invalid_argument if invalid string
 */
PROCESS_NATURE stringToProcessNature(const std::string& nature);


/* filePath: Path to JSON file
 
   Vector of Process pointers (allocated on heap)
   std::runtime_error if file cannot be opened
 
   */
std::vector<Process*> getProcessesFromJSON(const std::string& filePath);


#endif