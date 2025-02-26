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

// Function to generate random telemetry data
json generateTelemetryData()
{
    random_device rd;
    mt19937 gen(rd());

    // Define ranges for telemetry fields
    uniform_real_distribution<> gps_lat(-90.0, 90.0);   // Latitude range
    uniform_real_distribution<> gps_lon(-180.0, 180.0); // Longitude range
    uniform_real_distribution<> speed(0.0, 120.0);      // Speed range (km/h)
    uniform_real_distribution<> battery(0.0, 100.0);    // Battery percentage (0% - 100%)

    // Generate telemetry data
    json telemetry = {
        {"gps", {
                    {"latitude", gps_lat(gen)}, // Random latitude
                    {"longitude", gps_lon(gen)} // Random longitude
                }},
        {"speed", speed(gen)},    // Random speed
        {"battery", battery(gen)} // Random battery level
    };

    return telemetry; // Return telemetry as JSON
}

/**
int main()
{

    Testing:
     *
     *
        cout << "Starting telemetry simulation..." << endl;

        // Generate telemetry data in a loop
        for (int i = 0; i < 10; ++i) // Simulate 10 updates (modify as needed)
        {
            // Generate telemetry data
            json telemetry = generateTelemetryData();

            // Print telemetry data in JSON format
            cout << telemetry.dump(4) << endl; // Pretty-print with 4-space indentation

            // Wait for 1 second before generating the next telemetry data
            this_thread::sleep_for(chrono::seconds(1));
        }

        cout << "Telemetry simulation ended." << endl;

        return 0;


}
*/