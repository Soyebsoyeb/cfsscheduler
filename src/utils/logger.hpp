#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

// Used to manipulate cin , cout , file streams say by setprecision()
#include <iomanip>

// Helps to treat string as input/output stream
#include <sstream>
#include <iostream>
#include <map>


/**
 * Logger - Thread-safe logging system
 *  
 * Features:
  - Singleton pattern (single instance)
  - Thread-safe with mutex locks
  - Log levels: DEBUG, INFO, WARNING, ERROR
  - Output to both console and file
  - Timestamp for each log entry
  
 * Usage:
    Logger::info("Scheduler started");
    Logger::warning("Process queue is empty");
    Logger::error("Failed to allocate memory");
    Logger::debug("Process 1 vruntime: 1024");
 */



class Logger {
    private:
        // Get the singleton instance
        static Logger& getInstance() {
            // Thread safe static intializaiton
            static Logger instance; 

            return instance;
        }

        std::ofstream logFile;           // File output stream
        std::mutex logMutex;             // Mutex for Thread Safety
        std::string logLevel = "INFO";   // Current Log Level

        bool enabled = true;             // Enable / Disable Logging


        Logger(){

            // Open the file in append mode
            logFile.open("scheduler.log" , std::ios::app);

            if(!logFile.is_open()){
                enabled = false;
            }
        }

        // For current timestamp

        std::string getTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time) , "%Y-%m-%d %H:%M:%S" );

            return ss.str();
        }

        
        void log(const std::string& level , const std::string& message) {
            // Map log levels to priorities for filtering
            // Lower Number = Higher priority (DEBUG is lowest , ERROR is highest)
            
            static const std::map<std::string , int> levelPriority = {
                {"DEBUG", 0},    // Detailed debugging information
                {"INFO", 1},     // General information
                {"WARNING", 2},  // Warning messages
                {"ERROR", 3}     // Error messages
            };

            
            // Check if this message should be logged based on current log level

            auto currentPriority = levelPriority.find(logLevel);
            auto messagePriority = levelPriority.find(level);

            if (currentPriority == levelPriority.end() || 
                messagePriority == levelPriority.end() ||
                messagePriority->second < currentPriority->second) {
                return;  // Skip logging this message (level too low)
            }

            // Lock mutex for thread safety
             std::lock_guard<std::mutex> lock(logMutex);
        
            // Build the log entry with timestamp
            std::string logEntry = getTimestamp() + " [" + level + "] " + message;
        
            // Write to console
            std::cout << logEntry << std::endl;
        
            // Write to file if enabled
            if (enabled && logFile.is_open()) {
                logFile << logEntry << std::endl;
                logFile.flush();  // Ensure it's written immediately
            }

        }

    public:
    // Destructor - Closes the log file
     
        ~Logger() {
            if (logFile.is_open()) {
                logFile.close();
            }
        }
    
        // Public static logging methods
    
        // Log DEBUG-level message
        static void debug(const std::string& message) {
            getInstance().log("DEBUG", message);
        }
    
        // Log INFO-level message 
        static void info(const std::string& message) {
            getInstance().log("INFO", message);
        }
    
        // Log WARNING-level message (potential issues) 
        static void warning(const std::string& message) {
            getInstance().log("WARNING", message);
        }
    
        //Log ERROR-level message
        static void error(const std::string& message) {
            getInstance().log("ERROR", message);
        }
    
    
        // Set the minimum log level
        // Only messages with priority >= this level will be logged
     
        static void setLevel(const std::string& level) {
            getInstance().logLevel = level;
        }
    
    
        // Enable or disable logging
     
        static void enable(bool enable) {
            getInstance().enabled = enable;
        }
};

#endif