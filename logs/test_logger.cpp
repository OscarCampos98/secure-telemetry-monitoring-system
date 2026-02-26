#include "logger.h"
#include "../src/utils.h"
#include "../database/log_db_operations.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

int main()
{
    cout << "\n Running Logger Tests...\n"
         << endl;

    // Test 1: Basic Log Entry
    cout << "\n Test 1: Creating a Log Entry with a Real HMAC...\n"
         << endl;

    // create log JSON
    json logEntry = {
        {"component", "Test_Component"},
        {"message", "This is a test log entry"},
        {"log_level", "INFO"},
        {"timestamp", getCurrentTimestamp()},
    };

    // convert log JSON to a string
    string logString = logEntry.dump();

    // Get HMAC Key
    string hmacKey = getHMACKey();

    // Convert log string and key to unsigned char vector
    vector<unsigned char> logData(logString.begin(), logString.end());
    vector<unsigned char> keyData(hmacKey.begin(), hmacKey.end());

    // Generate real HMAC
    vector<unsigned char> computeHMAC = generateHMAC(logData, keyData);

    // convert HMAC to hex string
    ostringstream oss;
    for (unsigned char c : computeHMAC)
    {
        oss << hex << setw(2) << setfill('0') << (int)c;
    }

    string realHMAC = oss.str();

    cout << "DEBUG: Generated HMAC before insertion: " << realHMAC << endl;

    // Insert log entry into database
    bool success = insertLog("Test_Component", "This is a new test log entry", "INFO", realHMAC);
    if (success)
    {
        cout << "Test Log inserted entry successfully with real HMAC!\n";
    }
    else
    {
        cout << " Log entry insertion failed!\n";
    }

    // Fetch the most recent log ID dynamically
    cout << "\n Checking if log exists in database...\n"
         << endl;

    json lastLog;
    for (int id = 35; id >= 1; id--) // Start high and look for existing ID
    {
        lastLog = fetchLogById(id);
        if (!lastLog.empty() && lastLog.find("error") == lastLog.end())
        {
            cout << " Found latest log with ID: " << id << "\n";
            break;
        }
    }

    if (!lastLog.empty())
    {
        cout << lastLog.dump(4) << endl;
    }
    else
    {
        cout << "No log entry found in the database!\n";
    }

    cout << "DEBUG: Stored HMAC in database: " << lastLog["hmac"] << endl;

    // Test 2: Preventing Duplicate Logs
    cout << "\n Test 2: Ensuring Duplicate Prevention...\n"
         << endl;

    bool duplicateInserted = insertLog("Test_Component", "This is a new test log entry", "INFO", realHMAC);
    if (!duplicateInserted)
    {
        cout << " Duplicate log was correctly prevented!\n";
    }
    else
    {
        cout << " Duplicate log was inserted (this should not happen)!\n";
    }

    // Test 3: Fetch and Verify Log Integrity
    cout << "\n Test 3: Checking Log Integrity...\n"
         << endl;
    if (!lastLog.empty())
    {
        int logID = lastLog["id"].get<int>();
        if (verifyLogIntegrity(logID))
        {
            cout << " Log entry integrity verified successfully!\n";
        }
        else
        {
            cout << " Log entry integrity check FAILED!\n";
        }
    }
    else
    {
        cout << " Skipping integrity check since no log entry was found.\n";
    }

    cout << "\n Logger Tests Completed!\n"
         << endl;
    return 0;
}
