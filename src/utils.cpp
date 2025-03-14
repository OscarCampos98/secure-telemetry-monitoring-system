#include "utils.h"  // Include the header file for the utility functions
#include <iostream> // For console input and output
#include <iomanip>  // For formatting output (e.g., timestamp)
#include <chrono>   // For generating timestamps
#include <random>   // For random number generation
#include <string>   // For handling strings
#include "json.hpp" // JSON library for formatting and handling JSON data
#include "thread"

using namespace std;         // Use the standard namespace
using json = nlohmann::json; // Shortcut for easier JSON handling

// Function to generate the current timestamp in ISO 8601 format
string getCurrentTimestamp()
{
    // Get the current system time
    auto now = chrono::system_clock::now();

    // Convert system time to a time_t object for formatting
    auto now_time = chrono::system_clock::to_time_t(now);

    // Convert time_t to a UTC time structure
    tm utc_time = *gmtime(&now_time);

    // Format the time into an ISO 8601 string
    ostringstream oss;
    oss << put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ"); // e.g., "2024-12-05T14:23:05Z"
    return oss.str();                                 // Return the formatted timestamp as a string
}
