# secure-telemetry-monitoring-system

## Introduction

The **Secure Telemetry Monitoring System** is designed to provide **secure data transmission and monitoring** for uncrewed vehicles. It ensures the integrity and confidentiality of telemetry data while allowing real-time command execution and validation.

This system implements **AES-256 encryption** and **HMAC-SHA-256 authentication** to protect sensitive telemetry data and commands from tampering or interception. It also includes **LED indicators** to provide real-time feedback on the system’s status, helping operators quickly assess security and operational conditions.

The project is structured to be modular, making it **scalable and adaptable** to different uncrewed vehicle platforms.

## Features

The **Secure Telemetry Monitoring System** includes the following key features:

- **Secure Telemetry Data Generation**  
  - Simulates **GPS coordinates, speed, and battery levels**.  
  - Outputs data in **JSON format** with timestamps for accurate logging.

- **Command Processing & Validation**  
  - Supports commands like `SEND`, `RESEND`, `STATUS`, and `DECRYPT`.  
  - Ensures secure processing with **AES-256 encryption** and **HMAC validation**.  

- **Data Encryption & Integrity Verification**  
  - Utilizes **AES-256-CBC encryption** to protect telemetry data.  
  - **HMAC-SHA-256** prevents unauthorized message tampering.  

- **Real-Time LED Feedback (Optional)**  
  - **Green LED**: Indicates normal operation.  
  - **Red LED**: Signals errors or tampering detection.  
  - **Runs in two modes**:  
    - **With LEDs** (hardware connected to Raspberry Pi using a **T-Cobbler with LED connectors via GPIO**).  
    - **Without LEDs** (displays status in the terminal for systems without hardware support).  

- **Modular & Scalable Design**  
  - Can be adapted for various **uncrewed vehicle systems**.  
  - Designed for **Raspberry Pi** and compatible Linux environments.
 
  - ## System Components

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



