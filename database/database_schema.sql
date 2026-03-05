-- This SQL script creates the logs table for storing system logs.

-- Create the logs table if it doesn't already exist
CREATE TABLE IF NOT EXISTS logs (
    -- Unique identifier for each log entry (auto-increments)
    id SERIAL PRIMARY KEY,

    -- Timestamp when the log was created (defaults to the current time)
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    -- Name of the system component that generated the log entry (e.g., "Command", "Encryption", "Validation")
    component TEXT NOT NULL,

    -- The actual log message describing the event
    message TEXT NOT NULL,

    -- Log level indicating severity (e.g., "INFO", "WARNING", "ERROR")
    log_level TEXT NOT NULL,

    -- HMAC (Hash-based Message Authentication Code) to verify log integrity
    hmac TEXT NOT NULL

    
);
-- use payload id for hmac verification
ALTER TABLE logs 
ADD COLUMN IF NOT EXISTS payload TEXT NOT NULL;

-- Backfill legacy rows so we can enforce NOT NULL
UPDATE logs SET payload = '' WHERE payload IS NULL;

-- Enforce payload presence for all future rows
ALTER TABLE logs ALTER COLUMN payload SET NOT NULL;

--prevent duplicate logs with same component, message, log_level, 
CREATE UNIQUE INDEX IF NOT EXISTS LOGS_UNIQUE_ENTRY ON logs (component, message, log_level);

