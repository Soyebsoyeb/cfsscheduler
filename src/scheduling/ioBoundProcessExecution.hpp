#ifndef IO_BOUND_PROCESS_EXECUTION_HPP
#define IO_BOUND_PROCESS_EXECUTION_HPP

#include "../core/processService.hpp"
#include "queueService.hpp"
#include "cpuBoundProcessExecution.hpp"  // For weightFunction

/*
  Handle I/O-bound process execution
  
  I/O-bound processes:
  - Frequently wait for I/O (file, network, user input)
  - Are penalized in vruntime for waiting
  - This prevents CPU starvation by I/O-bound processes
  
 * Algorithm:
  1. Simulate I/O wait (sleep for ioWaitTime ms)
  2. Penalize vruntime for I/O wait time
  3. Execute CPU portion (1 time slice)
  4. Update vruntime for CPU portion
  5. If more work remains, requeue the process

 */
void handleIoBoundProcess(Process* process, int ioWaitTime, QueueService& queue);

#endif