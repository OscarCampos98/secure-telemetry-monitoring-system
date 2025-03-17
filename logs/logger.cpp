#include "logger.h"
#include "/home/pi/Desktop/secure-telemetry-monitoring-system/src/utils.h"
#include "/home/pi/Desktop/secure-telemetry-monitoring-system/src/encrypt_decrypt.h"

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

// Generic logging function
// note {} in arguments can't be set to defult value. move this to .h file.
void writeLog(const string &level, const string &component, const string &message, const json &extraData)
{
    lock_guard<mutex> lock(logMutex); // Ensure thread safety

    json logEntry = {
        {"timestamp", getCurrentTimestamp()},
        {"level", level},
        {"component", component},
        {"message", message},
        {"data", extraData}};

    // Convert log entry to string for hashing
    string LogString = logEntry.dump();

    // Retrive the HMAC key
    string hmacKey = getHMACKey();

    // Convert log enntry and key to unsigned char
    vector<unsigned char> logData(LogString.begin(), LogString.end());
    vector<unsigned char> KeyData(hmacKey.begin(), hmacKey.end());

    // Generate HMAC for the log entry
    vector<unsigned char> hmac = generateHMAC(logData, KeyData);

    // Convert HMAC to hex string
    ostringstream oss;
    for (unsigned char c : hmac)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    // Add HMAC to log entry
    logEntry["hmac"] = oss.str();

    // Print to console
    // cout << logEntry.dump(4) << endl;

    // Append to log file
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open())
    {
        logFile << logEntry.dump() << endl;
        logFile.close();
    }
}

// verify the HMAC of the log entry
bool verifyLogIntegrity(const json &logEntry)
{
    if (logEntry.find("hmac") == logEntry.end())
    {
        cout << "Log entry missing HMAC!" << endl;
        return false;
    }

    // Extract the HMAC from the log entry
    string storedHMAC = logEntry["hmac"];

    // Remove the HMAC from the log entry before verification
    json templog = logEntry;
    templog.erase("hmac");

    // convert log entry to string
    string logString = templog.dump();

    // retrive the HMAC key
    string hmacKey = getHMACKey();

    // Convert log entry and key to unsigned char
    vector<unsigned char> logData(logString.begin(), logString.end());
    vector<unsigned char> keyData(hmacKey.begin(), hmacKey.end());

    // Generate expected HMAC
    vector<unsigned char> expectedHMAC = generateHMAC(logData, keyData);

    // Convert HMAC to hex string
    ostringstream oss;
    for (unsigned char c : expectedHMAC)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    // Compare the HMAC with expected HMAC
    if (storedHMAC == oss.str())
    {
        cout << "Log entry is valid!" << endl;
        return true;
    }
    else
    {
        cout << "WARNING: Log entry may have been tampered with!" << endl;
        return false;
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

/* Testing purpose Main

int main()
{
    // Test current timestamp
    string timestamp = getCurrentTimestamp();
    cout << "Current timestamp: " << timestamp << endl;

    // Test logging different levels
    logInfo("Test_Component", "This is an INFO log message", {{"key", "value"}});
    logWarning("Test_Component", "This is a WARNING log message.", {{"key", "value"}});
    logError("Test_Component", "This is an ERROR log message.", {{"key", "value"}});
    logSecurity("Test_Component", "This is a SECURITY log message.", {{"key", "value"}});

    // Read the last log entry
    ifstream logFile("secure_monitoring.log");
    string lastLog;
    json logEntry;
    while (getline(logFile, lastLog))
    {
        logEntry = json::parse(lastLog);
    }
    logFile.close();

    // Verify log integrity
    if (verifyLogIntegrity(logEntry))
    {
        cout << "Log entry integrity verified!" << endl;
    }
    else
    {
        cout << "WARNING: Log entry has been tampered with!" << endl;
    }

    cout << "Logger test completed!" << endl;
    return 0;
}
*/