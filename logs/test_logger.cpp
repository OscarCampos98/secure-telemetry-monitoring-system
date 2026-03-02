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
    cout << "\nRunning Logger Tests...\n" << endl;

    // Test 1: Write a log through the logger pipeline
    cout << "Test 1: Writing a log via logInfo()...\n" << endl;

    logInfo("Test_Component", "This is a new test log entry", {{"test", true}});

    // Test 2: Fetch latest log and verify integrity
    cout << "\nTest 2: Fetch latest log and verify integrity...\n" << endl;

    json lastLog = fetchLatestLog();
    if (lastLog.contains("error"))
    {
        cout << "Failed to retrieve latest log: " << lastLog["error"] << endl;
        return 1;
    }

    cout << lastLog.dump(4) << endl;

    int logID = lastLog["id"].get<int>();
    cout << "\nVerifying integrity for log id: " << logID << endl;

    if (verifyLogIntegrity(logID))
        cout << "Integrity check PASSED.\n";
    else
        cout << "Integrity check FAILED.\n";

    cout << "\nLogger Tests Completed!\n" << endl;
    return 0;

}
