#include <iostream>
#include <pqxx/pqxx>         // PostgreSQL C++ library
#include <nlohmann/json.hpp> // JSON library for structured logs

using json = nlohmann::json;

using namespace std;
using namespace pqxx;

// Function to establish a connection to the database
// returns a pointer to a pqxx::connection object
connection *connectToDB()
{
    try
    {
        // Create a new connection instance with given cridentials
        connection *conn = new connection("dbname=secure_logging user=telemetry_user password=stms hostaddr=127.0.0.1 port=5432");

        // check if connection is open
        if (conn->is_open())
        {
            cout << "Connected to PostgreSQL database: " << conn->dbname() << endl;
        }
        else
        {
            cerr << "Failed to connect to the database!" << endl;
            delete conn; // Clean up memory if connection fails
            return nullptr;
        }

        return conn; // Return the connection object
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return nullptr;
    }
}

// Function to close the database connection
void closeDBConnection(connection *conn)
{
    if (conn)
    {
        conn->disconnect(); // perform disconnection
        delete conn;        // free memory allocation
        cout << "Connection closed successfully." << endl;
    }
}

bool logExists(const string &component, const string &message, const string &log_level)
{
    try
    {
        connection *conn = connectToDB();
        if (!conn)
        {
            cout << "DEGUG: logExists() - Connection failed!" << endl;
            return false;
        }

        nontransaction txn(*conn); // create a non-transaction object

        // SQL query to check if a log entry already exists
        string query = "SELECT COUNT(*) FROM logs WHERE component = " + txn.quote(component) +
                       " AND message = " + txn.quote(message) + " AND log_level = " + txn.quote(log_level);

        result res = txn.exec(query); // execute the query
        closeDBConnection(conn);      // close the connection

        int count = res[0][0].as<int>(); // returns true if log exists
        // cout << "DEBUG: logExists() - Count of existing log: " << count << endl;

        return count > 0;
    }
    catch (const exception &e)
    {
        cerr << "Error checking if log exists: " << e.what() << endl;
        return false;
    }
}

// Insert a new log entry
bool insertLog(const string &component, const string &message, const string &log_level, const string &hmac)
{
    try
    {
        if (logExists(component, message, log_level))
        {
            cout << "Log already exists! Skipping insertion" << endl;
            return false;
        }

        connection *conn = connectToDB();
        if (!conn)
            return false;

        work txn(*conn); // create a transaction object

        // SQL query to insert a new log entry
        string query = "INSERT INTO logs (component, message, log_level, hmac) VALUES (" +
                       txn.quote(component) + ", " + txn.quote(message) + ", " +
                       txn.quote(log_level) + ", " + txn.quote(hmac) + ")";
        txn.exec(query);         // execute the query
        txn.commit();            // commit the transaction
        closeDBConnection(conn); // close the connection

        // cout << "DEBUG: insertLog() - Log successfully inserted!" << endl;

        return true;
    }
    catch (const pqxx::unique_violation &e) // Catch unique constraint violation
    {
        cerr << "Error: Duplicate log entry detected! " << e.what() << endl;
        return false;
    }
    catch (const exception &e)
    {
        cerr << "Error inserting log: " << e.what() << endl;
        return false;
    }
}

// Delete a log entry by ID
bool deleteLog(int log_id)
{
    try
    {
        connection *conn = connectToDB();
        if (!conn)
            return false;

        work txn(*conn);
        string query = "DELETE FROM logs WHERE id = " + txn.quote(log_id);
        txn.exec(query);
        txn.commit();
        closeDBConnection(conn);
        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error deleting log: " << e.what() << endl;
        return false;
    }
}

// Update a log entry's message by ID
bool updateLog(int log_id, const string &new_message)
{
    try
    {
        connection *conn = connectToDB();
        if (!conn)
            return false;

        work txn(*conn);
        string query = "UPDATE logs SET message = " + txn.quote(new_message) + " WHERE id = " + txn.quote(log_id);
        txn.exec(query);
        txn.commit();
        closeDBConnection(conn);
        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error updating log: " << e.what() << endl;
        return false;
    }
}

// Fetch all logs (limit optional)
void fetchLogs(int limit = 10)
{
    try
    {
        connection *conn = connectToDB();
        if (!conn)
            return;

        nontransaction txn(*conn); // create a non-transaction object
        string query = "SELECT * FROM logs ORDER BY timestamp DESC LIMIT " + to_string(limit);
        result res = txn.exec(query); // execute the query

        // loop through the each row print log details
        for (const auto &row : res)
        {
            cout << "ID: " << row["id"].as<int>() << " | Component: " << row["component"].as<string>()
                 << " | Message: " << row["message"].as<string>() << " | Log Level: " << row["log_level"].as<string>()
                 << " | HMAC: " << row["hmac"].as<string>() << " | Timestamp: " << row["timestamp"].as<string>() << endl;
        }
        closeDBConnection(conn);
    }
    catch (const exception &e)
    {
        cerr << "Error fetching logs: " << e.what() << endl;
    }
}

json fetchLogById(int log_id)
{
    try
    {
        // cout << "DEBUG: Connecting to DB..." << endl;
        connection *conn = connectToDB();
        if (!conn)
        {
            // cout << "DEBUG: Connection failed." << endl;
            return json{{"error", "Connection failed"}};
        }

        // cout << "DEBUG: Running query..." << endl;
        nontransaction txn(*conn);
        string query = "SELECT * FROM logs WHERE id = " + to_string(log_id);
        result res = txn.exec(query);

        // cout << "DEBUG: Query executed. Rows returned: " << res.size() << endl;

        if (res.empty())
        {
            closeDBConnection(conn);
            return json{{"error", "Log not found"}}; // return errror JSON explicitly
        }

        const auto &row = res[0];

        // cout << "DEBUG: Fetching fields..." << endl;

        json logEntry = {
            {"id", row["id"].as<int>()},
            {"timestamp", row["timestamp"].as<string>()},
            {"component", row["component"].as<string>()},
            {"message", row["message"].as<string>()},
            {"log_level", row["log_level"].as<string>()},
            {"hmac", row["hmac"].as<string>()}};

        // cout << "DEBUG: Fields fetched successfully!" << endl;

        closeDBConnection(conn); // <--- Ensure we close after JSON construction

        // cout << "DEBUG: JSON Object Constructed: " << logEntry.dump(4) << endl;

        return logEntry; // Returning JSON safely
    }
    catch (const exception &e)
    {
        cerr << "Error fetching log by ID: " << e.what() << endl;
        return json{{"error", "Exception occurred"}};
    }
}
