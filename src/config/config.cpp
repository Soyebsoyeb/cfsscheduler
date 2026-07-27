#include "config.hpp"
#include <fstream>
#include <iostream>


Config::Config(const std::string& filename) {
    load(filename);
}

bool Config::load(const std::string& filename) {
    configFile = filename;
    loadFromFile(filename);
    return loaded;
}



void Config::loadFromFile(const std::string& filename) {
    try {
        // Open and parse the JSON file
        std::ifstream file(filename);
        if (!file.is_open()) {
            Logger::warning("Config file not found: " + filename);
            loaded = false;
            return;
        }
        
        json configJson;
        file >> configJson;  // Parse JSON
        file.close();
        
        // Parse scheduler configuration
        if (configJson.contains("scheduler")) {
            auto& sched = configJson["scheduler"];
            scheduler.timeSliceMs = sched.value("time_slice_ms", 2);
            scheduler.ioWaitMs = sched.value("io_wait_ms", 10);
            scheduler.nice0Load = sched.value("nice_0_load", 1024);
            scheduler.preemptive = sched.value("preemptive", true);
            scheduler.minGranularityMs = sched.value("min_granularity_ms", 1);
        }
        
        // Parse monitoring configuration
        if (configJson.contains("monitoring")) {
            auto& monitor = configJson["monitoring"];
            monitoring.enabled = monitor.value("enabled", true);
            monitoring.updateIntervalMs = monitor.value("update_interval_ms", 100);
            monitoring.port = monitor.value("port", 8080);
            monitoring.maxHistory = monitor.value("max_history", 1000);
            monitoring.websocketEnabled = monitor.value("websocket_enabled", true);
        }
        
        // Parse logging configuration
        if (configJson.contains("logging")) {
            auto& logging = configJson["logging"];
            logging.enabled = logging.value("enabled", true);
            logging.level = logging.value("level", "INFO");
            logging.outputFile = logging.value("output_file", "scheduler.log");
            
            // Update the logger with these settings
            Logger::setLevel(logging.level);
            Logger::enable(logging.enabled);
        }
        
        // Parse statistics configuration
        if (configJson.contains("statistics")) {
            auto& stats = configJson["statistics"];
            statistics.enabled = stats.value("enabled", true);
            statistics.outputFile = stats.value("output_file", "stats.json");
        }
        
        loaded = true;
        Logger::info("Configuration loaded from: " + filename);
        
    } catch (const std::exception& e) {
        Logger::warning("Failed to load config: " + std::string(e.what()));
        loaded = false;
    }
}


bool Config::save(const std::string& filename) const {
    try {
        // Create JSON object
        json configJson;
        
        // Add scheduler configuration
        configJson["scheduler"]["time_slice_ms"] = scheduler.timeSliceMs;
        configJson["scheduler"]["io_wait_ms"] = scheduler.ioWaitMs;
        configJson["scheduler"]["nice_0_load"] = scheduler.nice0Load;
        configJson["scheduler"]["preemptive"] = scheduler.preemptive;
        configJson["scheduler"]["min_granularity_ms"] = scheduler.minGranularityMs;
        
        // Add monitoring configuration
        configJson["monitoring"]["enabled"] = monitoring.enabled;
        configJson["monitoring"]["update_interval_ms"] = monitoring.updateIntervalMs;
        configJson["monitoring"]["port"] = monitoring.port;
        configJson["monitoring"]["max_history"] = monitoring.maxHistory;
        configJson["monitoring"]["websocket_enabled"] = monitoring.websocketEnabled;
        
        // Add logging configuration
        configJson["logging"]["enabled"] = logging.enabled;
        configJson["logging"]["level"] = logging.level;
        configJson["logging"]["output_file"] = logging.outputFile;
        
        // Add statistics configuration
        configJson["statistics"]["enabled"] = statistics.enabled;
        configJson["statistics"]["output_file"] = statistics.outputFile;
        
        // Write to file with pretty printing (4 spaces indentation)
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        
        file << configJson.dump(4);
        file.close();
        
        Logger::info("Configuration saved to: " + filename);
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to save config: " + std::string(e.what()));
        return false;
    }
}


void Config::print() const {
    std::cout << "\n=== Configuration ===" << std::endl;
    std::cout << "Scheduler:" << std::endl;
    std::cout << "  Time Slice: " << scheduler.timeSliceMs << "ms" << std::endl;
    std::cout << "  I/O Wait: " << scheduler.ioWaitMs << "ms" << std::endl;
    std::cout << "  Nice 0 Load: " << scheduler.nice0Load << std::endl;
    std::cout << "  Preemptive: " << (scheduler.preemptive ? "Yes" : "No") << std::endl;
    std::cout << "  Min Granularity: " << scheduler.minGranularityMs << "ms" << std::endl;
    
    std::cout << "\nMonitoring:" << std::endl;
    std::cout << "  Enabled: " << (monitoring.enabled ? "Yes" : "No") << std::endl;
    std::cout << "  Update Interval: " << monitoring.updateIntervalMs << "ms" << std::endl;
    std::cout << "  Port: " << monitoring.port << std::endl;
    std::cout << "  Max History: " << monitoring.maxHistory << std::endl;
    std::cout << "  WebSocket: " << (monitoring.websocketEnabled ? "Enabled" : "Disabled") << std::endl;
    
    std::cout << "\nLogging:" << std::endl;
    std::cout << "  Enabled: " << (logging.enabled ? "Yes" : "No") << std::endl;
    std::cout << "  Level: " << logging.level << std::endl;
    std::cout << "  Output: " << logging.outputFile << std::endl;
    
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "  Enabled: " << (statistics.enabled ? "Yes" : "No") << std::endl;
    std::cout << "  Output: " << statistics.outputFile << std::endl;
}