#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <string>
#include "json.hpp"

// Function prototypes
std::string getCurrentTimestamp();
nlohmann::json generateTelemetryData();

#endif // TELEMETRY_H
