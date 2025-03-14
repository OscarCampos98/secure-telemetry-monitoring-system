#include "led_control.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h> // For sleep function

using namespace std;

// GPIO pin definitions
const string GREEN_LED = "19"; // GPIO 19 (Green LED)
const string RED_LED = "26";   // GPIO 26 (Red LED)

void writeToFile(const string &path, const string &value)
{
    ofstream file(path);
    if (!file)
    {
        cerr << "Error: Unable to write to " << path << endl;
        throw runtime_error("Failed to write to " + path); // Exit the program if writing fails
    }
    file << value;
    file.close();
}

void exportGPIO(const string &pin)
{
    cout << "Exporting GPIO " << pin << " ..." << endl;
    // check if the GPIO pin is already exported
    ifstream gpioCheck("/sys/class/gpio/gpio" + pin);
    if (gpioCheck.good())
    {
        cout << "GPIO " << pin << "already exported." << endl;
        gpioCheck.close();
        return;
    }
    gpioCheck.close();

    // export the pin
    writeToFile("/sys/class/gpio/export", pin);
    // Wait briefly to allow the system to initialize the pin
    usleep(100000); // 100ms delay

    cout << "GPIO " << pin << " successfully exported." << endl;
}

void setDirection(const string &pin, const string &direction)
{
    string path = "/sys/class/gpio/gpio" + pin + "/direction";
    for (int i = 0; i < 3; i++)
    {
        try
        {
            writeToFile(path, direction);
            cout << "Direction for GPIO " << pin << " set to" << direction << "." << endl;
            return;
        }
        catch (...)
        {
            cerr << "Retrying to set direction for GPIO " << pin << " ..." << endl;
            usleep(100000); // wait for 100ms before retrying
        }
    }
    throw runtime_error("Failed to set direction for GPIO " + pin);
}

void setValue(const string &pin, const string &value)
{
    writeToFile("/sys/class/gpio/gpio" + pin + "/value", value);
}

void unexportGPIO(const string &pin)
{
    cout << "Unexporting GPIO " << pin << " ..." << endl;
    writeToFile("/sys/class/gpio/unexport", pin);
}

// define LED control functions
void allOff()
{
    setValue(GREEN_LED, "0");
    setValue(RED_LED, "0");
}

void normalOperation()
{
    for (int i = 0; i < 3; i++)
    {
        setValue(GREEN_LED, "1"); // Green On
        setValue(RED_LED, "0");   // Red Off
        sleep(1);
    }
    allOff();
}

void errorDetected()
{
    setValue(GREEN_LED, "0"); // Green Off
    for (int i = 0; i < 3; i++)
    {
        setValue(RED_LED, "1"); // Red On
        sleep(1);
        setValue(RED_LED, "0"); // Red Off
        sleep(1);
    }
    allOff();
}

void tamperingDetected()
{
    setValue(GREEN_LED, "0");
    for (int i = 0; i < 3; i++)
    {
        setValue(RED_LED, "1"); // Red On
        usleep(500000);         // Delay for 0.5 seconds
        setValue(RED_LED, "0"); // Red Off
        usleep(500000);         // Delay for 0.5 seconds
    }
    allOff();
}

/**
 * //TEST

int main()
{
    // Export GPIO pins
    exportGPIO(GREEN_LED);
    exportGPIO(RED_LED);

    // Set directions
    setDirection(GREEN_LED, "out");
    setDirection(RED_LED, "out");

    cout << "LED controller ready. Waiting for command.." << endl;

    string command;

    try
    {
        while (true)
        {
            cin >> command;

            if (command == "NORMAL")
            {
                cout << "Executing NORMAL operation ..." << endl;
                normalOperation();
            }
            else if (command == "ERROR")
            {
                cout << "Executing Error indication... " << endl;
                errorDetected();
            }
            else if (command == "TAMPER")
            {
                cout << "Executing TAMPER detection... " << endl;
                tamperingDetected();
            }
            else if (command == "OFF")
            {
                cout << "Turning OFF all LEDS... " << endl;
                allOff();
            }
            else if (command == "EXIT")
            {
                cout << "Exiting LED controller... " << endl;
                break;
            }
            else
            {
                cout << "unkown command..." << endl;
            }
        }
    }
    catch (...)
    {
        cerr << "An error occurred." << endl;
    }

    // Cleanup
    unexportGPIO(GREEN_LED);
    unexportGPIO(RED_LED);

    return 0;
}
*/