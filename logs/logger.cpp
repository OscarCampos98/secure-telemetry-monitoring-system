#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "logger.h"
#include "../src/utils.h"
#include "../src/encrypt_decrypt.h"
#include "../database/log_db_operations.h"

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

    // Store log PostgreSQL database
    if (!insertLog(component, message, level, oss.str()))
    {
        cout << "Error inserting log into database! Failing back to file-only logging." << endl;
        logError("Logger", "Database insert failed for log entry!", {{"component", component}, {"message", message}});
    }
    else
    {
        cout << "log successfully stored in database." << endl;
    }

    // Append to log file
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open())
    {
        logFile << logEntry.dump() << endl;
        logFile.close();
    }
}

// verify the HMAC of the log entry using database
bool verifyLogIntegrity(int log_id)
{

    // Fetch the log entry from the database
    json logEntry = fetchLogById(log_id);

    // **Debug Output: Print the JSON**
    cout << "DEBUG: logEntry contents: " << logEntry.dump(4) << endl;

    if (logEntry.empty() || logEntry.find("hmac") == logEntry.end())
    {
        cout << "Log entry missing HMAC in database! Checking file log..." << endl;

        // Attempt to retrive from file
        ifstream logFile(LOG_FILE);
        string lastLog;
        while (getline(logFile, lastLog))
        {
            json fileLog = json::parse(lastLog);
            if (fileLog["id"] == log_id)
            {
                logEntry = fileLog;
                break;
            }
        }
        logFile.close();

        if (logEntry.empty())
        {
            cout << "Log entry not found in database or file! Integrity verification failed. " << endl;
            logSecurity("Logger", "Log entry missing in both database and file!", {{"log_id", log_id}});
            return false;
        }
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
        logSecurity("Logger", "Log entry verification failed!", {{"log_id", log_id}});
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
