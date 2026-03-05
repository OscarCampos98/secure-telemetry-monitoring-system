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
    
    json logEntryObj = {
        {"timestamp", getCurrentTimestamp()},
        {"component", component},
        {"log_level", level},
        {"message", message},
        {"data", extraData}};

    string logEntryStr = logEntryObj.dump();

    string hmacKey = getHMACKey();
    cout << "DEBUG: HMAC key used for logging: " << hmacKey << endl;

    vector<unsigned char> Data(logEntryStr.begin(), logEntryStr.end());

    vector<unsigned char> KeyData = hexStringToBytes(hmacKey);

    cout << "DEBUG: logEntry string used for HMAC (logging): " << logEntryStr << endl;

    vector<unsigned char> hmac = generateHMAC(Data, KeyData);
    
    ostringstream oss;
    for (unsigned char c : hmac)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    //Store in DB
    bool db_ok = insertLog(component, message, level, logEntryStr ,oss.str());
    if((!db_ok))
    {
        cerr << "DB insert failed; writing to file only. "
                << "Component=" << component << " level=" << level << endl;
    }else{
        cout << "Log entry successfully stored in database." << endl;
    }

    //Always right to file (fallback)
    ofstream logFile(LOG_FILE, ios::app);
    if (logFile.is_open())
    {
        json fileEntry ={
            {"payload", logEntryObj}, 
            {"hmac", oss.str()}
        };
        logFile << fileEntry.dump() << endl;
        logFile.close();
    }
}

bool verifyLogIntegrity(int log_id)
{
    json logEntry = fetchLogById(log_id);
    cout << "DEBUG: Retrieved log entry: " << logEntry.dump(4) << endl;

    if (logEntry.empty() || logEntry.contains("error"))
    {
        cerr << "verifyLogIntegrity: Log not found or DB error." << endl;
        return false;
    }

    if (!logEntry.contains("payload") || !logEntry.contains("hmac"))
    {
        cerr << "verifyLogIntegrity: Missing payload or hmac in DB row." << endl;
        return false;
    }

    string payload = logEntry["payload"].get<string>();
    string storedHMAC = logEntry["hmac"].get<string>();

    if(payload.empty())
    {
        cerr << "verifyLogIntegrity: Log ID " << log_id 
             << " is a legacy row with empty payload; cannot verify integrity." << endl;
        return false;
    }

    string hmacKey = getHMACKey();
    cout << "DEBUG: HMAC key used for verification: " << hmacKey << endl;
    vector<unsigned char> Data(payload.begin(), payload.end());
    vector<unsigned char> keyData = hexStringToBytes(hmacKey);
    vector<unsigned char> expectedHMAC = generateHMAC(Data, keyData);

    ostringstream oss;
    for (unsigned char c : expectedHMAC)
        oss << hex << setw(2) << setfill('0') << (int)c;

    string recomputed = oss.str();

    cout << "DEBUG: Stored HMAC: " << storedHMAC << endl;
    cout << "DEBUG: Recomputed HMAC: " << recomputed << endl;

    if(storedHMAC == recomputed)
    {
        cout << "Log entry is valid!" << endl;
        return true;
    }
   
    cout << "WARNING: Log entry verification failed!" << endl;
    return false;
    

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
