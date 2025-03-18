#include <iostream>
#include "log_db_operations.h"
#include <nlohmann/json.hpp> // JSON library

using json = nlohmann::json;
using namespace std;

int main()
{
    cout << "Testing log database operations...\n"
         << endl;

    string testComponent = "Test_Component";
    string testMessage = "This is a test log message";
    string testLogLevel = "INFO";
    string testHmac = "dummy_hmac";

    // Insert a test log
    cout << "Inserting a test log..." << endl;
    if (insertLog(testComponent, testMessage, testLogLevel, testHmac))
    {
        cout << "Test log inserted successfully!" << endl;
    }
    else
    {
        cout << "Failed to insert test log!" << endl;
    }

    // Fetch all logs
    cout << "\nFetching all logs (limit 5)..." << endl;
    fetchLogs(5);

    // Fetch the latest log entry
    cout << "\nFetching the latest inserted log..." << endl;
    json lastLog = fetchLogById(5); // Assuming last inserted log ID is 2 for now

    cout << "DEBUG: JSON received from fetchLogById:\n"
         << lastLog.dump(4) << endl;

    // Check if lastLog contains an error
    if (lastLog.find("error") != lastLog.end())
    {
        cout << "Failed to retrieve last inserted log: " << lastLog["error"] << ". Exiting test." << endl;
        return 1;
    }

    cout << "DEBUG: Extracting log ID..." << endl;
    int testLogId = lastLog["id"].get<int>(); // Fix: Only one declaration

    cout << "DEBUG: Log ID extracted: " << testLogId << endl;

    // Fetch log by ID again
    cout << "\nFetching log by ID: " << testLogId << endl;
    json logEntry = fetchLogById(testLogId);

    // Check if logEntry contains an error
    if (logEntry.find("error") != logEntry.end())
    {
        cout << "Failed to fetch log: " << logEntry["error"] << endl;
        return 1;
    }

    cout << "Fetched Log Data: " << logEntry.dump(4) << endl;

    // Update a log message
    cout << "\nUpdating log message with ID: " << testLogId << endl;
    if (updateLog(testLogId, "Updated test log entry."))
    {
        cout << "Log updated successfully!" << endl;
    }
    else
    {
        cout << "Failed to update log!" << endl;
    }

    // Fetch the updated log
    cout << "\nFetching updated log by ID: " << testLogId << endl;
    logEntry = fetchLogById(testLogId);
    if (logEntry.find("error") != logEntry.end())
    {
        cout << "Failed to fetch updated log: " << logEntry["error"] << endl;
    }
    else
    {
        cout << "Updated log:\n"
             << logEntry.dump(4) << endl;
    }

    // Insert a duplicate log to test duplicate prevention
    cout << "\nTrying to insert the same log again..." << endl;
    if (insertLog(testComponent, testMessage, testLogLevel, testHmac))
    {
        cout << "Duplicate log was inserted! (This should not happen)" << endl;
    }
    else
    {
        cout << "Duplicate log was correctly prevented!" << endl;
    }

    // Delete the test log
    cout << "\nDeleting test log with ID: " << testLogId << endl;
    if (deleteLog(testLogId))
    {
        cout << "Test log deleted successfully!" << endl;
    }
    else
    {
        cout << "Failed to delete test log!" << endl;
    }

    cout << "\nTesting completed!" << endl;
    return 0;
}
