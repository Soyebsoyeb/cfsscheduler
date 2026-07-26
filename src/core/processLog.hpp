#ifndef PROCESS_LOG_HPP
#define PROCESS_LOG_HPP

/**
 * ProcessLog - Records a single execution episode
 * 
 * Each time a process runs, we create one of these
 * This allows us to reconstruct the entire schedule timeline
 * 
 * Used for:
 * - Generating CSV output for visualization
 * - Analyzing scheduler behavior
 * - Debugging scheduling decisions
 */

 struct ProcessLog {
    int pid;
    long long startTime;
    long long endTime;

    // Duration of this execution episode can 
    //   be easily calculated using endTime - startTime
    //   thus they can be easily computed
 };

#endif