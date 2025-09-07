# secure-telemetry-monitoring-system
> 🚧 Prototype project — core encryption + telemetry pipeline working; logging & network transmission in progress.

## Introduction

Goal: Secure, tamper-resistant telemetry + command channel for Raspberry Pi–based uncrewed systems.

Security: AES-256-CBC + HMAC-SHA-256.

Feedback: LED status + planned PostgreSQL logs.

## Features

### ✅ Current
- Telemetry data generator (GPS, speed, battery).
- AES-256-CBC encryption + HMAC validation.
- LED feedback (GPIO 19 = green, 26 = red).
- Command set: SEND, RESEND, STATUS, DECRYPT.

### 📝 Planned/Roadmap
- [ ] Integrate PostgreSQL logging (schema defined).
- [ ] Logger module (tamper-resistant logs with HMAC).
- [ ] Automated integration tests (main.cpp).
- [ ] Remote telemetry transmission.

 
## System Components

The **Secure Telemetry Monitoring System** consists of multiple components that work together for **secure telemetry generation, encryption, validation, and monitoring**.

### **1. `main.cpp` - Core System Controller**
- Manages **command processing** and **LED feedback**.
- Detects if **GPIO hardware (T-Cobbler with LEDs via GPIO)** is available.
- Handles user interaction, including:
  - Checking system **STATUS**.
  - Sending encrypted telemetry (`SEND`).
  - Resending last message (`RESEND`).
  - Decrypting data (`DECRYPT`).
- Provides **real-time status updates** either through **LED indicators or terminal output**.

### **2. `encrypt_decrypt.cpp` - Secure Communication**
- Implements **AES-256-CBC encryption** to protect telemetry data.
- Uses **SHA-256 hashing with HMAC** to prevent message tampering.
- Encrypts messages before transmission and **decrypts/validates** upon reception.
- Generates cryptographic **keys and initialization vectors (IVs)** securely.

### **3. `telemetry.cpp` - Telemetry Data Generator**
- Simulates real-time telemetry data, including:
  - **GPS coordinates**
  - **Speed**
  - **Battery level**
- Outputs telemetry in **JSON format** with accurate timestamps.
- Supports **automatic telemetry generation loops** for testing.

### **4. `led_control.cpp` - LED Status Indicator**
- Controls **real-time LED feedback** to indicate system status.
- Uses **Raspberry Pi GPIO via a T-Cobbler with LED connectors**:
  - **Green LED (GPIO 19)** - Normal operation.
  - **Red LED (GPIO 26)** - Error/tampering detection.
- Implements:
  - **normalOperation()** → Lights up Green LED.
  - **errorDetected()** → Blinks Red LED for **errors**.
  - **tamperingDetected()** → Blinks Red LED for **tampering**.
  - **allOff()** → Turns off all LEDs.

### **5. `toHexString.cpp` - Hash Conversion Utility**
- Converts **binary SHA-256 hash values** into human-readable hexadecimal strings.
- Used for verifying message integrity after decryption.

### **6. Header Files (`include/` folder)**
- `encrypt_decrypt.h` - Function prototypes for encryption/decryption.
- `telemetry.h` - Telemetry data structures and function declarations.
- `led_control.h` - LED control function definitions.
- `toHexString.h` - Utility function prototypes for hash conversion.

### **7. `Makefile` - Build Automation**
- Defines rules for compiling the program.
- Ensures all required files are built correctly.

## Directory Structure
```bash
secure-telemetry-monitoring-system/
│-- src/                # Source files
│   │-- main.cpp
│   │-- encrypt_decrypt.cpp
│   │-- telemetry.cpp
│   │-- led_control.cpp
│-- include/            # Header files
│   │-- encrypt_decrypt.h
│   │-- telemetry.h
│   │-- led_control.h
│-- logging/            # Log storage (future implementation)
│-- Makefile            # Build automation
│-- README.md           # Project documentation
```

## Logging System
The logging system will be implemented to track key events for debugging, security monitoring, and system performance analysis.
### Current Readiness for Logging:
We have already prepared for logging in the following ways:
Error Handling & Debugging: cerr is used throughout the code to capture encryption failures, decryption errors, and MAC validation issues.
```bash
cerr << "Error during decryption: " << e.what() << endl;
```

Console Output for Status Messages: System operations such as encryption results, telemetry generation, and LED feedback are printed to the console.

```bash
cout << "System Status: " << message.dump(4) << endl;
```

Planned Directory for Logs: A logging/ directory is included in the project structure for future log storage.
Error Handling & Debugging: cerr is used throughout the code to capture encryption failures, decryption errors, and MAC validation issues.

### Planned Logging Features
- Events to Log:
  - Telemetry generation (telemetry.cpp)
  - Encryption & decryption actions (encrypt_decrypt.cpp)
  - MAC validation results (encrypt_decrypt.cpp)
  - Security alerts for tampering (main.cpp)
  - Command executions (main.cpp) 
  - System errors (any file)

### Implementation Plan:
- A new module (logger.cpp and logger.h) will be created to manage logging.
- Logs will be stored in logging/system.log. A log_event() function will be integrated across key parts of the project.
- Logging will be lightweight to avoid performance issues.


## Build & Run 
This section provides details on how to compile and execute the Secure Telemetry Monitoring System.

### Prerequisites:
Before building and running the system, ensure the following dependencies are installed:
 - Operating System: Linux (Raspberry Pi recommended)
 - Compiler: GCC (GNU Compiler Collection)
Libraries:
 - OpenSSL (for encryption and hashing)
 - nlohmann/json.hpp (for JSON handling)
 - To install the required dependencies on a Debian-based system (e.g., Raspberry Pi OS, Ubuntu):
```
bash
sudo apt update
sudo apt install build-essential libssl-dev
```
### Build Instructions:
The system uses a Makefile for automated compilation.
To build the project, navigate to the project directory and run:

```bash
make
```

This will generate an executable named secure_telemetry in the project root.

### Run Instructions
After building, execute the system using:
```bash
./STMonitoring
```

### Execution Options
When prompted, indicate whether GPIO hardware is available for LED feedback.
If using a Raspberry Pi with a T-Cobbler setup, ensure the LEDs are connected to the correct GPIO pins before running.

### Cleaning Up
To remove compiled binaries, use:
```
make clean
```

## Testing 
The system has undergone testing in individual components, and test cases are now being developed for main.cpp to validate the complete system integration.

### Completed Component Testing
 - encrypt_decrypt.cpp: Encryption, decryption, and MAC validation were tested using a standalone main function.
 - telemetry.cpp: Telemetry data generation was verified by running multiple iterations and checking JSON output     
   consistency.
 - led_control.cpp: LED functionality was validated separately using command-based testing in its own main function.
 - Ongoing System-Wide Testing (main.cpp)
    - Test cases are being developed for main.cpp, where all components work together. These tests will ensure:
 - Correct command execution (SEND, RESEND, STATUS, DECRYPT)
 - Encryption and decryption work seamlessly within the full system
 - MAC validation correctly detects tampering
 - LED indicators respond appropriately to different system states
 - Error handling and invalid input detection function as expected
### Future Enhancements
 - Automating test cases to validate system behavior under different conditions
 - Introducing logging-based validation to track unexpected failures
 - Expanding tests to include simulated network transmission for remote telemetry








## Troubleshooting 

## Future Enhancements


