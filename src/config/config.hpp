#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "../../include/nlohmann/json.hpp"
#include "../utils/logger.hpp"

using json = nlohmann::json;



// SchedulerConfig - Configuration for the CFS scheduler
struct SchedulerConfig {
    int timeSliceMs = 2;          // CPU time slice in milliseconds
    int ioWaitMs = 10;            // I/O wait time in milliseconds
    int nice0Load = 1024;         // Standard Linux nice 0 weight
    bool preemptive = true;       // Enable/disable preemption
    int minGranularityMs = 1;     // Minimum scheduling granularity
};


//MonitorConfig - Configuration for monitoring
struct MonitorConfig {
    bool enabled = true;          // Enable/disable monitoring
    int updateIntervalMs = 100;   // Update interval in milliseconds
    int port = 8080;              // Port for web dashboard
    int maxHistory = 1000;        // Maximum historical data points
    bool websocketEnabled = true; // Enable WebSocket support
};


// LoggingConfig - Configuration for logging
struct LoggingConfig {
    bool enabled = true;          // Enable/disable logging
    std::string level = "INFO";   // Log level: DEBUG, INFO, WARNING, ERROR
    std::string outputFile = "scheduler.log";  // Log file path
};


// StatsConfig - Configuration for statistics
struct StatsConfig {
    bool enabled = true;          // Enable/disable statistics
    std::string outputFile = "stats.json";  // Statistics output file
};

/* Config - Main configuration class
  
  Loads configuration from JSON file using nlohmann/json
  All values have default settings if file not found
 */


class Config {
private:
    std::string configFile;      // Configuration file path
    bool loaded = false;         // Whether config was loaded
    

    void loadFromFile(const std::string& filename);

public:
    SchedulerConfig scheduler;    // Scheduler configuration
    MonitorConfig monitoring;     // Monitoring configuration
    LoggingConfig logging;        // Logging configuration
    StatsConfig statistics;       // Statistics configuration
    
    
    Config(const std::string& filename = "scheduler_config.json");

    
    ~Config() = default;
    
    // Load configuration from file
    bool load(const std::string& filename);
    
    // Save current configuration to file
    bool save(const std::string& filename) const;
    
    // Check if configuration was loaded
    bool isLoaded() const { return loaded; }
    
    //Print configuration to console
    void print() const;
};

#endif