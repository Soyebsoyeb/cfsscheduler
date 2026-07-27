#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>

#include "src/core/processService.hpp"
#include "src/core/processLog.hpp"
#include "src/scheduling/cfs.hpp"
#include "src/monitoring/monitor.hpp"
#include "src/config/config.hpp"
#include "src/utils/logger.hpp"

/**
 * main() - Entry point of the program
 * 
 * Workflow:
 * 1. Load configuration from JSON file
 * 2. Load processes from JSON file
 * 3. Create scheduler and monitor
 * 4. Start monitoring thread
 * 5. Run scheduler
 * 6. Stop monitoring
 * 7. Save results to CSV files
 * 8. Print statistics
 * 9. Clean up memory
 */
int main(int argc, char* argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    
    // Print welcome message
    std::cout << "=== CFS Scheduler with Real-Time Monitoring ===" << std::endl;
    
    // Step 1: Load configuration from scheduler_config.json
    Config config("scheduler_config.json");
    config.print();  // Display loaded configuration
    
    // Step 2: Load processes from JSON file
    std::vector<Process*> processes;
    
    try {
        processes = getProcessesFromJSON("../resources/process.json");
        Logger::info("Successfully loaded " + std::to_string(processes.size()) + " processes");
    } catch (const std::exception& e) {
        Logger::error("Failed to load processes: " + std::string(e.what()));
        return 1;
    }
    
    // Step 3: Create scheduler and monitor
    cfs scheduler(config);
    Monitor monitor(config.monitoring);
    
    // Register a callback function for monitoring updates
    monitor.registerCallback([](const MonitorData& data) {
        static int count = 0;
        count++;
        
        if (count % 10 == 0) {
            std::cout << "\r[Monitor] Processes: " << data.queueSize 
                     << " | Current: " << data.currentProcess
                     << " | Switches: " << data.contextSwitches
                     << " | CPU: " << data.totalCpuTime << "ms" << std::flush;
        }
    });
    
    // Step 4: Start monitoring thread
    Logger::info("Starting monitoring thread...");
    std::thread monitorThread([&monitor, &scheduler]() {
        monitor.start(scheduler);
    });
    
    // Step 5: Run the scheduler
    Logger::info("Starting scheduler...");
    std::cout << "\nStarting CFS Scheduler with Real-Time Monitoring..." << std::endl;
    std::cout << "Monitoring updates every " << config.monitoring.updateIntervalMs << "ms" << std::endl;
    
    // schedule() runs the main scheduling loop
    // Returns vector of execution logs
    auto logs = scheduler.schedule(processes, &monitor);
    
    // Step 6: Stop monitoring
    Logger::info("Stopping monitor...");
    monitor.stop();
    monitorThread.join();
    
    // Step 7: Save results to CSV files
    Logger::info("Saving results...");
    
    // Save schedule timeline to CSV
    std::ofstream outFile("process_schedule.csv");
    outFile << "pid,start_time,end_time" << std::endl;
    for (auto log : logs) {
        outFile << log->pid << ","
                << log->startTime << ","
                << log->endTime << std::endl;
    }
    outFile.close();
    Logger::info("Schedule saved to process_schedule.csv");
    
    // Save monitoring data to CSV
    monitor.saveToFile("monitoring_data.csv");
    
    // Step 8: Print final statistics
    std::cout << "\n\n";
    scheduler.printStatistics();
    
    // Step 9: Clean up dynamically allocated memory
    Logger::info("Cleaning up...");
    
    // IMPORTANT: The scheduler takes ownership of all processes.
    // It deletes them when they complete or when the scheduler is destroyed.
    // Clear the vector to prevent double deletion.
    processes.clear();
    
    // Delete execution logs (scheduler does NOT delete these)
    for (auto log : logs) {
        delete log;
    }
    
    Logger::info("Program completed successfully");
    return 0;
}