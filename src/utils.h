#ifndef UTILS_H
#define UTILS_H

#include <iostream> // For console input and output
#include <iomanip>  // For formatting output (e.g., timestamp)
#include <chrono>   // For generating timestamps
#include <random>   // For random number generation
#include <string>   // For handling strings
#include "json.hpp" // JSON library for formatting and handling JSON data
#include "thread"

std::string getCurrentTimestamp();

#endif // UTILS_H