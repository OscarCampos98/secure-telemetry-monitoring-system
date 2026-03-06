#include "telemetry.h"
#include "encrypt_decrypt.h"
#include "led_control.h"
#include "utils.h"
#include "../logs/logger.h"

#include <iostream>
#include <fstream>
#include <thread>    //for std::this_thread::sleep_for
#include <chrono>    //for std::chrono::seconds
#include <nlohmann/json.hpp>  // for JSON handling
#include <termios.h> //for checking key press
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <limits>

using namespace std;
using json = nlohmann::json;

vector<unsigned char> lastCiphertext; // Stores the last encrypted message
string lastCommand;   // NEW: Stores the last executed command
bool hasGPIO = false; // NEW: Indicates if GPIO pins are available

// Function to check if GPIO pins are available
bool checkGPIOAvailability()
{
    return access("/dev/gpiochip0", R_OK | W_OK) == 0;
}

bool initializeGPIO()
{
    try
    {
        exportGPIO(GREEN_LED);
        exportGPIO(RED_LED);
        setDirection(GREEN_LED, "out");
        setDirection(RED_LED, "out");
        return true;
    }
    catch (const exception &e)
    {
        cerr << "GPIO initialization failed: " << e.what() << endl;
        cerr << "Falling back to text-based indicators." << endl;
        return false;
    }
}

// Function to enable non-blocking input detection
bool keyPressed()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt); // Get terminal attributes
    newt = oldt;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set new terminal attributes
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Set non-blocking input

    ch = getchar(); // Get character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Reset terminal attributes
    fcntl(STDIN_FILENO, F_SETFL, oldf);      // restore old input settings

    if (ch != EOF) // If character is detected
    {
        ungetc(ch, stdin); // Put character back
        // cout << "Key detected!" << endl;
        return true; // Return true
    }

    return false;
}

// Timer for breadboard set up 5 minutes time. Press any key to continue earlier
// Timer will be change to 15 seconds for testing
void waitForSetup()
{
    cout << "Waiting for 5 minutes. Press any key to continue earlier..." << endl;
    for (int i = 6; i > 0; --i)
    {
        cout << "Time remaining: " << i << " seconds\r";
        cout.flush();
        this_thread::sleep_for(chrono::seconds(1));

        if (keyPressed()) // Detects if any key is pressed
        {
            cout << "Key detected!" << endl;
            break;
        }
    }
    cout << "Timer completed or interrupted. Proceeding..." << endl; // Ensure this prints
}

// Function to update JSON with status and MAC validation
json updateJSON(const string &originalMessage, const string &status, bool macValid)
{

    json parsed = json::parse(originalMessage);
    parsed["status"] = status;
    parsed["MAC Validation"] = macValid;
    return parsed;
}

void handleCommand(const string &command, string &lastFormattedMessage, vector<unsigned char> &key, vector<unsigned char> &iv, vector<unsigned char> &macKey)
{
    logInfo("Command", "Received command: " + command, {{"command", command}});
    cout << "Executing command: " << command << endl;

    if (command == "SEND")
    {
        // Generate telemetry data and format message
        string formattedMessage = formatString("SEND");
        lastFormattedMessage = formattedMessage;

        // Encrypt the full JSON message
        vector<unsigned char> ciphertext = macAndEncrypt(formattedMessage, key, iv, macKey);
        lastCiphertext = ciphertext;

        // logging
        logInfo("Encryption", "Message encrypted successfully.",
                {{"ciphertext_len", (int)ciphertext.size()}});

        // Output results (removed: toHexString(ciphertext))
        cout << "Message Sent. Ciphertext:" << endl;

        /*TEST
        cout << "Key: " << toHexString(key) << endl;
        cout << "IV: " << toHexString(iv) << endl;
        cout << "MAC Key: " << toHexString(macKey) << endl;
        cout << "\n"
             << endl;
        */

        // Indicate error via LED
        if (hasGPIO)
            normalOperation();
        else
            cout << "Result: valid(G)" << endl;

        lastCommand = "SEND"; // Update last command
    }
    else if (command == "RESEND")
    {
        if (lastFormattedMessage.empty())
        {
            logError("Resend", "No message to resend.", {{"command", command}});
            cout << "No message to resend!" << endl;
            if (hasGPIO)
                errorDetected();
            else
                cout << "Result: invalid (R)" << endl;
        }
        else
        {
            // Reuse last formatted message
            vector<unsigned char> ciphertext = macAndEncrypt(lastFormattedMessage, key, iv, macKey);
            lastCiphertext = ciphertext;
            logInfo("Encryption", "Message encrypted successfully.",
                     {{"ciphertext_len", (int)ciphertext.size()}});
            cout << "Re-sent Message." << endl;

            if (hasGPIO)
                normalOperation();
            else
                cout << "Result: valid (G)" << endl;
            lastCommand = "RESEND"; // Update last command

            /*TESTING
            cout << "Ciphertext: " << toHexString(lastCiphertext) << endl;
            cout << "Key: " << toHexString(key) << endl;
            cout << "IV: " << toHexString(iv) << endl;
            cout << "MAC Key: " << toHexString(macKey) << endl;
            cout << "\n"
                 << endl;
            */
        }
    }
    else if (command == "STATUS")
    {
        if (lastFormattedMessage.empty())
        {
            logWarning("Command", "STATUS requested, but no message was sent.", {{"command", command}});
            cout << "STATUS requested, but no message was sent." << endl;

            if (hasGPIO)
            {
                errorDetected();
                allOff();
            }
            else
            {
                cout << "Result: invalid (R)" << endl;
            }
        }
        else
        {
            vector<unsigned char> data = decrypt(lastCiphertext, key, iv);

            vector<unsigned char> extractedMAC(data.begin(), data.begin() + 32);
            vector<unsigned char> extractedHash(data.begin() + 32, data.begin() + 64);
            vector<unsigned char> extractedPlaintext(data.begin() + 64, data.end());

            vector<unsigned char> macData(extractedHash.begin(), extractedHash.end());
            macData.insert(macData.end(), extractedPlaintext.begin(), extractedPlaintext.end());

            bool result = validateMAC(macData, macKey, extractedMAC);

            json parsedMessage = json::parse(lastFormattedMessage);
            json message = {
                {"command", parsedMessage["command"]},
                {"last_command", lastCommand},
                {"timestamp", parsedMessage["timestamp"]},
                {"MAC Validation", result},
                {"status", result ? "Normal" : "Tamper"}};

            logInfo("Validation", "Message check performed.", message);
            cout << "System Status: " << message.dump(4) << endl;

            if (result)
            {
                if (hasGPIO)
                    normalOperation();
                else
                    cout << "Result: valid (G)" << endl;
            }
            else
            {
                if (hasGPIO)
                    tamperingDetected();
                else
                    cout << "Result: invalid (R) - Tampering Detected " << endl;
            }
        }
    }

    else if (command == "DECRYPT")
    {
        if (lastFormattedMessage.empty())
        {
            logWarning("Decryption", "No message to decrypt!", {{"command", command}});
            cout << "No message to decrypt!" << endl;
            // Indicate error via LED
            if (hasGPIO)
            {
                errorDetected();
                this_thread::sleep_for(chrono::seconds(3));
                allOff();
            }
            else
            {
                cout << "Result: invalid (R)" << endl;
                this_thread::sleep_for(chrono::seconds(3));
            }
            return;
        }

        /* TESTING Values

        cout << "Decrypting message..." << endl;
        cout << "Ciphertext: " << toHexString(lastCiphertext) << endl;
        cout << "Key: " << toHexString(key) << endl;
        cout << "IV: " << toHexString(iv) << endl;
        cout << "MAC Key: " << toHexString(macKey) << endl;
        cout << "\n"
             << endl;
        */

        try
        {

            DecryptionResult result = decryptAndVerify(lastCiphertext, key, iv, macKey);

            /*TESTING
            cerr << "Post-decrypt in main.cpp..." << endl;
            cerr << "Plaintext size: " << result.plaintext.size() << endl;
            cerr << "MAC Valid: " << (result.macValid ? "True" : "False") << endl;
            cerr << "Timestamp: " << result.timestamp << endl;
            cerr << "Hash (Hex): " << toHexString(result.hash) << endl;
            */

            // Display MAC validation status
            if (result.macValid)
            {
                logInfo("Decryption", "Decryption successful.",
                        {{"mac_valid", result.macValid},
                            {"plaintext_len", (int)result.plaintext.size()},
                            {"timestamp", result.timestamp}});

                cout << "Decryption successful." << endl;
                // Indicate error via LED
                if (hasGPIO)
                    normalOperation();
                else
                    cout << "Result: valid (G)" << endl;
            }
            else if (result.plaintext.empty())
            {
                logError("Decryption", "Decryption failed: empty plaintext.", {{"command", command}});
                cout << "Decryption failed: empty plaintext." << endl;
                if (hasGPIO)
                    errorDetected();
                else
                    cout << "Result: invalid (R)" << endl;
                return;
            }
            else
            {
                logSecurity("Decryption", "MAC validation failed! Possible tampering detected.",
                            {{"mac_valid", result.macValid},
                                {"plaintext_len", (int)result.plaintext.size()},
                                {"timestamp", result.timestamp}});
                cout << "MAC validation failed! Possible tampering detected" << endl;
                // Indicate error via LED
                if (hasGPIO)
                    tamperingDetected();
                else
                    cout << "Result: invalid (R) - Tampering Detected" << endl;
            }
        }
        catch (const std::exception &e)
        {
            cerr << "Decryption failed: " << e.what() << endl;
            if (hasGPIO)
                errorDetected();
            else
                cout << "Result: invalid (R)" << endl;
        }
    }

    /* Not needed
    else if (command == "OFF")
    {
        allOff();
    }
    */

    else if (command == "EXIT")
    {
        logInfo("Command", "Exiting program...", {});
        cout << "Exiting program..." << endl;

        if (hasGPIO)
        {
            allOff();
            unexportGPIO(GREEN_LED);
            unexportGPIO(RED_LED);
        }

        exit(0);
    }
    else
    {
        logWarning("Command", "Unknown command: " + command, {{"command", command}});
        cout << "Unknown command: " << command << endl;
    }
}

int main()
{
    // User preferences for the system
    char userResponse;
    bool validResponce = false;

    while (!validResponce)
    {
        cout << "Do you have a GPIO breadboard setup? (y/n): ";
        cin >> userResponse;

        if (userResponse == 'y' || userResponse == 'Y')
        {
            if (checkGPIOAvailability())
            {
                cout << "GPIO interface detected." << endl;
                cout << "Green LED GPIO Pin: 19" << endl;
                cout << "Red LED GPIO Pin: 26" << endl;

                char setupResponse;
                cout << "Do you need 5 minutes to set up the breadboard? (y/n): ";
                cin >> setupResponse;

                if (setupResponse == 'y' || setupResponse == 'Y')
                {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    waitForSetup();
                }

                cout << "Proceeding to initialize GPIO setup..." << endl;
                hasGPIO = initializeGPIO();

                if (hasGPIO)
                {
                    cout << "GPIO initialized successfully. Using GPIO LEDs." << endl;
                }
                else
                {
                    cout << "GPIO unavailable. Using text-based indicators instead." << endl;
                }
            }
            else
            {
                cout << "GPIO interface not accessible. Falling back to text output." << endl;
                hasGPIO = false;
            }

            validResponce = true;
        }
        else if (userResponse == 'n' || userResponse == 'N')
        {
            cout << "User opted out of GPIO. Using text-based indicators." << endl;
            hasGPIO = false;
            validResponce = true;
        }
        else
        {
            cout << "Invalid response, please enter 'y' or 'n'." << endl;
        }
    }

    // Generate function
    vector<unsigned char> key = generateRandomKey(32);
    vector<unsigned char> iv = generateRandomKey(16);
    vector<unsigned char> macKey = generateRandomKey(32);

    string command;
    string lastFormattedMessage;

    try
    {
        while (true)
        {
            cout << "> ";
            cin >> command;
            handleCommand(command, lastFormattedMessage, key, iv, macKey);
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        if (hasGPIO)
        {
            allOff();
            unexportGPIO(GREEN_LED);
            unexportGPIO(RED_LED);
        }
    }

    return 0;
}