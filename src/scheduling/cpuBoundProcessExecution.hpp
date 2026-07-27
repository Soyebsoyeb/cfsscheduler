#ifndef CPU_BOUND_PROCESS_EXECUTION_HPP
#define CPU_BOUND_PROCESS_EXECUTION_HPP

#include "../core/processService.hpp" 
#include "queueService.hpp"            

/*  Calculate weight from priority
  
  In CFS, weight determines how fast vruntime grows:
  - Higher priority → higher weight → slower vruntime growth
  - This means higher priority processes get more CPU time
  
   Formula: weight = NICE_0_LOAD / (priority + 1)
  
 * NICE_0_LOAD = 1024 (standard Linux value)
 

 */
double weightFunction(int priority);

/* Execute a CPU-bound process for one time slice
  
 * CPU-bound processes:
  - Use CPU continuously (no I/O waiting)
  - Are executed for the full time slice
  - Have vruntime increased proportionally to execution time
  
 * Algorithm:
  1. Determine execution time = min(timeSlice, remaining_burst)
  2. Reduce remaining CPU time
  3. Calculate weight from priority
  4. Update vruntime: += (executedTime * NICE_0_LOAD) / weight
  5. If more work remains, requeue the process

 */
void executeCpuBoundProcess(Process* process, int timeSlice, QueueService& queue);

#endif