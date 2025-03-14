#include "logger.h"
#include "/home/pi/Desktop/secure-telemetry-monitoring-system/src/utils.h"

#include <iostream>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp> // JSON library for structured logs
#include <sys/stat.h>        // For checking file size

using json = nlohmann::json;
using namespace std;

// Global log file name
const string LOG_FILE = "secure_monitoring.log";

// Mutex for thread safety
mutex logMutex;

/**
 * Check the size of the log file and rotate if necessary
 * @return file size in bytes, or -1 if the file does not exist
 */
long getLogFileSize()
{
    struct stat fileStat;
    if (stat(LOG_FILE.c_str(), &fileStat) == 0)
    {
        return fileStat.st_size; // return file size in bytes
    }
    return -1; // File does not exist or can't be access.
}

// Generic logging function
void writeLog(const string &level, const string &component, const string &message, const json &extraData = {})
{
    lock_guard<mutex> lock(logMutex); // Ensure thread safety

    json logEntry = {
        {"timestamp", getCurrentTimestamp()},
        {"level", level},
        {"component", component},
        {"message", message},
        {"data", extraData}};

    // Print to console
    cout << logEntry.dump(4) << endl;

    // Append to log file
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open())
    {
        logFile << logEntry.dump() << endl;
        logFile.close();
    }
}

// Public log functions
void logInfo(const string &component, const string &message, const json &extraData)
{
    writeLog("INFO", component, message, extraData);
}

void logWarning(const string &component, const string &message, const json &extraData)
{
    writeLog("WARNING", component, message, extraData);
}

void logError(const string &component, const string &message, const json &extraData)
{
    writeLog("ERROR", component, message, extraData);
}

void logSecurity(const string &component, const string &message, const json &extraData)
{
    writeLog("SECURITY", component, message, extraData);
}

int main()
{
    // Test log file size
    long fileSize = getLogFileSize();
    cout << "Log file size: " << fileSize << " bytes" << endl;

    // Test current timestamp
    string timestamp = getCurrentTimestamp();
    cout << "Current timestamp: " << timestamp << std::endl;

    // Test logging different levels
    logInfo("Test_Component", "This is an INFO log message", {{"key", "value"}});
    logWarning("Test_Component", "This is a  WARNING log message.", {{"key", "value"}});
    logError("Test_Component", "This is an ERROR log message.", {{"key", "value"}});
    logSecurity("Test_Component", "This is a SECURITY log message.", {{"key", "value"}});

    cout << "Logger test completed!" << endl;
    return 0;
}