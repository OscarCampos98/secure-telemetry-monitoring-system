#include "logger.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp> // JSON library for structured logs

using json = nlohmann::json;

// Global log file name
const std::string LOG_FILE = "/var/log/secure_monitoring.log";

// Mutex for thread safety
std::mutex logMutex;

// Function to get current timestamp in ISO 8601 format
std::string getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tmStruct = *std::localtime(&timeT);
    std::ostringstream oss;
    oss << std::put_time(&tmStruct, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// Generic logging function
void writeLog(const std::string &level, const std::string &component, const std::string &message, const json &extraData = {})
{
    std::lock_guard<std::mutex> lock(logMutex); // Ensure thread safety

    json logEntry = {
        {"timestamp", getTimestamp()},
        {"level", level},
        {"component", component},
        {"message", message},
        {"data", extraData}};

    // Print to console
    std::cout << logEntry.dump(4) << std::endl;

    // Append to log file
    std::ofstream logFile(LOG_FILE, std::ios::app);
    if (logFile.is_open())
    {
        logFile << logEntry.dump() << std::endl;
        logFile.close();
    }
}

// Public log functions
void logInfo(const std::string &component, const std::string &message, const json &extraData)
{
    writeLog("INFO", component, message, extraData);
}

void logWarning(const std::string &component, const std::string &message, const json &extraData)
{
    writeLog("WARNING", component, message, extraData);
}

void logError(const std::string &component, const std::string &message, const json &extraData)
{
    writeLog("ERROR", component, message, extraData);
}

void logSecurity(const std::string &component, const std::string &message, const json &extraData)
{
    writeLog("SECURITY", component, message, extraData);
}
