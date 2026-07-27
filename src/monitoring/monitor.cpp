#include "monitor.hpp"
#include "../scheduling/cfs.hpp"  
#include "../utils/logger.hpp"
#include <fstream>
#include <chrono>
#include <thread>

Monitor::Monitor(const MonitorConfig& cfg) : config(cfg) {
    Logger::info("Monitor initialized");
}

Monitor::~Monitor() {
    stop();
}


void Monitor::start(cfs& scheduler) {
    (void)scheduler;  // Suppress unused parameter warning
    
    if (running) return;  // Already running
    
    running = true;
    
    // Create background thread
    monitorThread = std::thread([this]() {
        Logger::info("Monitor thread started");
        
        while (running) {
            // Sleep for the configured interval
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.updateIntervalMs));
            
            // Lock data for thread safety
            std::lock_guard<std::mutex> lock(dataMutex);
            
            // Broadcast current data to callbacks
            broadcastData(currentData);
            
            // Save to history
            history.push_back(currentData);
            if (history.size() > static_cast<size_t>(config.maxHistory)) {
                history.erase(history.begin());
            }
        }
    });
    
    Logger::info("Monitoring started");
}

void Monitor::stop() {
    if (!running) return;
    
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();  // Wait for thread to finish
    }
    
    Logger::info("Monitoring stopped");
}

void Monitor::update(const MonitorData& data) {
    std::lock_guard<std::mutex> lock(dataMutex);
    currentData = data;
}

void Monitor::broadcastData(const MonitorData& data) {
    for (const auto& callback : callbacks) {
        try {
            callback(data);  // Call the callback
        } catch (const std::exception& e) {
            Logger::error("Callback error: " + std::string(e.what()));
        }
    }
}

void Monitor::registerCallback(std::function<void(const MonitorData&)> callback) {
    callbacks.push_back(callback);
    Logger::info("New monitoring callback registered");
}

MonitorData Monitor::getCurrentData() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return currentData;
}

std::vector<MonitorData> Monitor::getHistory() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return history;
}

void Monitor::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        Logger::error("Cannot save monitoring data to: " + filename);
        return;
    }
    
    // Write CSV header
    file << "timestamp,currentProcess,queueSize,contextSwitches,totalCpuTime\n";
    
    // Write each data point
    for (const auto& data : history) {
        file << data.timestamp << ","
             << data.currentProcess << ","
             << data.queueSize << ","
             << data.contextSwitches << ","
             << data.totalCpuTime << "\n";
    }
    
    file.close();
    Logger::info("Monitoring data saved to: " + filename);
}