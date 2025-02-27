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

