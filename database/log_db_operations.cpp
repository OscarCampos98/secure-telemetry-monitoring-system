#include <pqxx/pqxx>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

using namespace std;
using namespace pqxx;

//Conection string helper 
static std::string getConnString()
{
    return "dbname=secure_logging user=telemetry_user password=REDACTED hostaddr=127.0.0.1 port=5432";
}

static bool logExists(const std::string &component,
                      const std::string &message,
                      const std::string &log_level)
{
    try
    {
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return false;

        int count = 0;
        {
            pqxx::nontransaction txn(conn);

            std::string query =
                "SELECT COUNT(*) FROM logs WHERE component = " + txn.quote(component) +
                " AND message = " + txn.quote(message) +
                " AND log_level = " + txn.quote(log_level);

            pqxx::result res = txn.exec(query);
            if (!res.empty())
                count = res[0][0].as<int>(0);
        }

        return count > 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error checking if log exists: " << e.what() << std::endl;
        return false;
    }
}

// Insert a new log entry
bool insertLog(const string &component,
               const string &message,
               const string &log_level,
               const string &payload,
               const string &hmac)
{
    try
    {
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return false;

        {
            pqxx::work txn(conn);

            string query =
                "INSERT INTO logs (component, message, log_level, payload, hmac) VALUES (" +
                txn.quote(component) + ", " +
                txn.quote(message) + ", " +
                txn.quote(log_level) + ", " +
                txn.quote(payload) + ", " +
                txn.quote(hmac) + ")";

            txn.exec(query);
            txn.commit();
        }

        return true;
    }
    catch (const pqxx::unique_violation &e)
    {
        cerr << "Duplicate prevented by DB: " << e.what() << endl;
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
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return false;

        {
            pqxx::work txn(conn);
            string query = "DELETE FROM logs WHERE id = " + txn.quote(log_id);
            txn.exec(query);
            txn.commit();
        }

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
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return false;

        {
            pqxx::work txn(conn);
            string query =
                "UPDATE logs SET message = " + txn.quote(new_message) +
                " WHERE id = " + txn.quote(log_id);

            txn.exec(query);
            txn.commit();
        }

        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error updating log: " << e.what() << endl;
        return false;
    }
}

// Fetch all logs (limit optional)
void fetchLogs(int limit)
{
    try
    {
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
        {
            cerr << "Failed to connect to DB for fetchLogs()." << endl;
            return;
        }

        {
            pqxx::nontransaction txn(conn);
            string query =
                "SELECT * FROM logs ORDER BY timestamp DESC LIMIT " + to_string(limit);

            pqxx::result res = txn.exec(query);

            for (const auto &row : res)
            {
                cout
                    << "ID: " << row["id"].as<int>()
                    << " | Component: " << row["component"].as<string>()
                    << " | Message: " << row["message"].as<string>()
                    << " | Log Level: " << row["log_level"].as<string>()
                    << " | HMAC: " << row["hmac"].as<string>()
                    << " | Timestamp: " << row["timestamp"].as<string>()
                    << endl;
            }
        }
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
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return json{{"error", "Connection failed"}};

        json logEntry;

        {
            pqxx::nontransaction txn(conn);
            std::string query = "SELECT * FROM logs WHERE id = " + txn.quote(log_id);
            pqxx::result res = txn.exec(query);

            if (res.empty())
            {
                logEntry = json{{"error", "Log not found"}};
            }
            else
            {
                const auto &row = res[0];
                logEntry = {
                    {"id", row["id"].as<int>()},
                    {"timestamp", row["timestamp"].as<std::string>()},
                    {"component", row["component"].as<std::string>()},
                    {"message", row["message"].as<std::string>()},
                    {"log_level", row["log_level"].as<std::string>()},
                    {"payload", row["payload"].as<std::string>()},
                    {"hmac", row["hmac"].as<std::string>()}};
            }
        }

        return logEntry;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching log by ID: " << e.what() << std::endl;
        return json{{"error", "Exception occurred"}};
    }
}

json fetchLatestLog()
{
    try
    {
        pqxx::connection conn(getConnString());
        if (!conn.is_open())
            return json{{"error", "Connection failed"}};

        json logEntry;

        {
            pqxx::nontransaction txn(conn);
            pqxx::result res = txn.exec("SELECT * FROM logs ORDER BY id DESC LIMIT 1");

            if (res.empty())
            {
                logEntry = json{{"error", "No logs found"}};
            }
            else
            {
                const auto &row = res[0];
                logEntry = {
                    {"id", row["id"].as<int>()},
                    {"timestamp", row["timestamp"].as<std::string>()},
                    {"component", row["component"].as<std::string>()},
                    {"message", row["message"].as<std::string>()},
                    {"log_level", row["log_level"].as<std::string>()},
                    {"payload", row["payload"].as<std::string>()},
                    {"hmac", row["hmac"].as<std::string>()}};
            }
        }

        return logEntry;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching latest log: " << e.what() << std::endl;
        return json{{"error", "Exception occurred"}};
    }
}
        
        
 
