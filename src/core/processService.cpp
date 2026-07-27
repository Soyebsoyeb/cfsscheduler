#include "processService.hpp"

// Convert string to PROCESS_NATURE enum

PROCESS_NATURE stringToProcessNature(const std::string& nature) {
    if (nature == "CPU_BOUND") return PROCESS_NATURE::CPU_BOUND;
    if (nature == "IO_BOUND") return PROCESS_NATURE::IO_BOUND;
    throw std::invalid_argument("Invalid PROCESS_NATURE value: " + nature);
}

//  Load processes from JSON file using nlohmann/json
 
std::vector<Process*> getProcessesFromJSON(const std::string& filePath) {
    
    // Step 1: Open and read the file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
    
    // Step 2: Parse JSON 
    json processesJson;
    file >> processesJson;  // Parse into JSON object
    file.close();
    
    // Step 3: Validate the structure
    if (!processesJson.is_array()) {
        throw std::runtime_error("JSON root must be an array");
    }
    
    // Step 4: Convert each JSON object to a Process
    std::vector<Process*> processes;
    
    for (const auto& item : processesJson) {
        // Allocate new Process on the heap
        Process* process = new Process();
        
        // Extract fields using .get<T>() for type safety
        process->pid = item["pid"].get<int>();
        process->vruntime = item["vruntime"].get<long long>();
        process->cpu_burst_time = item["cpu_burst_time"].get<int>();
        process->priority = item["priority"].get<int>();
        process->processNature = stringToProcessNature(
            item["processNature"].get<std::string>()
        );
        
        // Optional field with default
        if (item.contains("io_burst_time")) {
            process->io_burst_time = item["io_burst_time"].get<int>();
        } else {
            process->io_burst_time = 0;  // Default
        }
        
        // Initialize state
        process->state.state = READY;
        process->state.lastStateChange = 0;
        
        processes.push_back(process);
    }
    
    // Log success
    Logger::info("Loaded " + std::to_string(processes.size()) + 
                 " processes from JSON: " + filePath);
    
    return processes;
}