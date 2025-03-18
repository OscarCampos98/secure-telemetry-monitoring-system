#ifndef LOG_DB_OPERATIONS_H
#define LOG_DB_OPERATIONS_H

#include <pqxx/pqxx> // PostgreSQL library
#include <iostream>
#include <nlohmann/json.hpp> // JSON library

using json = nlohmann::json;
using namespace pqxx;
using namespace std;

// Function to establish a PostgreSQL connection
pqxx::connection *connectToDB();

// Function to close the PostgreSQL connection
void closeDBConnection(pqxx::connection *conn);

// function to insert a new log entry into database
bool insertLog(const string &component, const string &message, const string &log_level, const string &hmac);

// function to delete a log entry by ID
bool deleteLog(int log_id);

// function to update a log entry's message
bool updateLog(int log_id, const string &message);

// function to fecch all logs (default limit: 10)
void fetchLogs(int limit = 10);

// functions to fetch a specific log entry by ID
json fetchLogById(int log_id);

#endif // LOG_DB_OPERATIONS_H
