#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp> // JSON library

using namespace std;
using json = nlohmann::json; // Define json type

// Function declarations
void writeLog(const string &level, const string &component, const string &message, const json &extraData = {});
bool verifyLogIntegrity(const json &logEntry);

// Public log functions
void logInfo(const string &component, const string &message, const json &extraData);
void logWarning(const string &component, const string &message, const json &extraData);
void logError(const string &component, const string &message, const json &extraData);
void logSecurity(const string &component, const string &message, const json &extraData);
#endif // LOGGER_H
