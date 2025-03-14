#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <string>
// GPIO pin definitions
const std::string GREEN_LED = "19"; // GPIO 19 (Green LED)
const std::string RED_LED = "26";   // GPIO 26 (Red LED)

// GPIO control function prototypes
void writeToFile(const std::string &path, const std::string &value);
void exportGPIO(const std::string &pin);
void setDirection(const std::string &pin, const std::string &direction);
void setValue(const std::string &pin, const std::string &value);
void unexportGPIO(const std::string &pin);

// LED control function prototypes
void normalOperation();
void errorDetected();
void tamperingDetected();
void allOff();

#endif // LED_CONTROL_H
