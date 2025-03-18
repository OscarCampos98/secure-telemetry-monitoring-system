#include "logger.h"
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
    cout << "\n Test 1: Creating a Log Entry...\n"
         << endl;
    logInfo("Test_Component", "This is a new test log entry", {{"key", "value"}});

    // Fetch the most recent log ID dynamically
    cout << "\n Checking if log exists in database...\n"
         << endl;

    json lastLog;
    for (int id = 10; id >= 1; id--) // Start high and look for existing ID
    {
        lastLog = fetchLogById(id);
        if (!lastLog.empty())
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

    // Test 2: Preventing Duplicate Logs
    cout << "\n Test 2: Ensuring Duplicate Prevention...\n"
         << endl;
    bool duplicateInserted = insertLog("Test_Component", "This is a new test log entry", "INFO", "test_hmac");
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
