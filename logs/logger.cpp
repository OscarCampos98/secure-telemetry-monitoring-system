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
void writeLog(const string &level, const string &component, const string &message, const json &extraData)
{
    lock_guard<mutex> lock(logMutex); // Ensure thread safety

    json logEntry = {
        {"timestamp", getCurrentTimestamp()},
        {"level", level},
        {"component", component},
        {"message", message},
        {"data", extraData}};

    string LogString = logEntry.dump();

    string hmacKey = getHMACKey();
    cout << "DEBUG: HMAC key used for logging: " << hmacKey << endl;

    vector<unsigned char> logData(LogString.begin(), LogString.end());
    vector<unsigned char> KeyData = hexStringToBytes(hmacKey);

    cout << "DEBUG: Log string used for HMAC (logging): " << LogString << endl;

    vector<unsigned char> hmac = generateHMAC(logData, KeyData);

    ostringstream oss;
    for (unsigned char c : hmac)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    logEntry["hmac"] = oss.str();

    if (!insertLog(component, message, level, oss.str()))
    {
        cerr << "Error inserting log into database! Falling back to file logging." << endl;
        logError("Logger", "Database insert failed for log entry!", {{"component", component}, {"message", message}});
    }
    else
    {
        cout << "Log successfully stored in database." << endl;
    }

    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open())
    {
        logFile << logEntry.dump() << endl;
        logFile.close();
    }
}

bool verifyLogIntegrity(int log_id)
{
    json logEntry = fetchLogById(log_id);
    cout << "DEBUG: Retrieved log entry: " << logEntry.dump(4) << endl;

    if (logEntry.empty() || logEntry.find("hmac") == logEntry.end())
    {
        cout << "Log entry missing HMAC! Checking file log..." << endl;
        return false;
    }

    string storedHMAC = logEntry["hmac"];
    json templog = logEntry;
    templog.erase("hmac");
    string logString = templog.dump();

    string hmacKey = getHMACKey();
    cout << "DEBUG: HMAC key used for verification: " << hmacKey << endl;

    vector<unsigned char> logData(logString.begin(), logString.end());
    vector<unsigned char> keyData = hexStringToBytes(hmacKey);

    cout << "DEBUG: Log string used for HMAC (verification): " << logString << endl;

    vector<unsigned char> expectedHMAC = generateHMAC(logData, keyData);

    ostringstream oss;
    for (unsigned char c : expectedHMAC)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    cout << "DEBUG: Stored HMAC: " << storedHMAC << endl;
    cout << "DEBUG: Recomputed HMAC: " << oss.str() << endl;

    if (storedHMAC == oss.str())
    {
        cout << "Log entry is valid!" << endl;
        return true;
    }
    else
    {
        cout << "WARNING: Log entry verification failed!" << endl;
        return false;
    }
}

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
