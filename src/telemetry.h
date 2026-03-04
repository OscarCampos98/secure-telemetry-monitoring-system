#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <string>
#include <nlohmann/json.hpp>

// Function prototypes
// std::string getCurrentTimestamp();
nlohmann::json generateTelemetryData();

#endif // TELEMETRY_H
