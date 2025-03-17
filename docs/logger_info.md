# Logger System Overview 

## Objective 
The logging system for the Secure Communication and Monitoring System aims to:
- Provide structured logging for debugging, monitoring, and security auditing.
- Support multiple log levels (INFO, WARNING, ERROR, SECURITY).
- Write logs to both a file and console for real-time and persistent storage.
- Ensure thread safety to prevent log corruption in multi-threaded operations.
- Format logs in JSON for easy parsing and analysis.
- Enable future enhancements like log rotation and remote log streaming.

## Features 
- Structured Logging: Logs are stored in JSON format with timestamps.
- Multiple Log Levels: INFO, WARNING, ERROR, SECURITY to categorize events.
- Thread Safety: Uses mutex locking to prevent race conditions.
- Dual Output: Logs are written to both console and a log file (/var/log/secure_monitoring.log).
- Error Handling: Ensures failures in logging do not crash the system.
- Extensibility: Can be expanded to include log rotation and remote logging.

## Log Format
Each log entry follows a structured format:
```bash
{
    "timestamp": "2025-03-13T14:30:00Z",
    "level": "INFO",
    "component": "telemetry",
    "message": "Telemetry data generated",
    "data": {
        "latitude": 51.0486,
        "longitude": -114.0708,
        "speed": 10.5,
        "battery": 85}
}
```

## Implementation Plan 
1. Create logger.cpp and logger.h with structured logging functions.
2. Implement log levels: INFO, WARNING, ERROR, SECURITY.
3. Ensure thread safety using std::mutex.
4. Integrate logging into project files:
    - main.cpp: Logs command execution.
    - encrypt_decrypt.cpp: Logs encryption/decryption events.
    - telemetry.cpp: Logs telemetry data generation.
    - led_control.cpp: Logs LED status changes.
5. Secure the logs
    - Restrict log file access permissions.
    - Enable append-only mode to prevent tampering.
    - Implement integrity verification using hashing.
    - Encrypt sensitive log fields if required.
    - Enable log rotation for better storage management.
6. Test logging functionality to verify:
    - Correct log format.
    - Proper file and console output.
    - Multi-threaded behavior.
7. Enhance logging with optional log rotation for better log management.
