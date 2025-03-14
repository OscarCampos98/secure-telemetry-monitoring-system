#include "telemetry.h"
#include "encrypt_decrypt.h"
#include "led_control.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <thread>    //for std::this_thread::sleep_for
#include <chrono>    //for std::chrono::seconds
#include "json.hpp"  //for JSON handling
#include <termios.h> //for checking key press
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <limits>

using namespace std;
using json = nlohmann::json;

string lastFormattedMessage;          // Stores the last formatted JSON message
vector<unsigned char> lastCiphertext; // Stores the last encrypted message
vector<unsigned char> key;
vector<unsigned char> iv;
vector<unsigned char> macKey;
string lastCommand;   // NEW: Stores the last executed command
bool hasGPIO = false; // NEW: Indicates if GPIO pins are available

// Function to check if GPIO pins are available
bool checkGPIOAvailability()
{
    ifstream gpioTest("/sys/class/gpio/");
    return gpioTest.good();
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
    if (command == "SEND")
    {
        // Generate telemetry data and format message
        string formattedMessage = formatString("SEND");
        lastFormattedMessage = formattedMessage;

        // Encrypt the full JSON message
        vector<unsigned char> ciphertext = macAndEncrypt(formattedMessage, key, iv, macKey);
        lastCiphertext = ciphertext;

        // Output results
        cout << "Message Sent." << endl;
        cout << "Ciphertext (Hex): " << toHexString(ciphertext) << endl;

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
            cerr << "No message to resend!" << endl;

            // Indicate error via LED or manually
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
            cout << "Re-sent Message." << endl;
            cout << "Ciphertext (Hex): " << toHexString(ciphertext) << endl;

            /*TESTING
            cout << "Ciphertext: " << toHexString(lastCiphertext) << endl;
            cout << "Key: " << toHexString(key) << endl;
            cout << "IV: " << toHexString(iv) << endl;
            cout << "MAC Key: " << toHexString(macKey) << endl;
            cout << "\n"
                 << endl;
            */

            // Indicate error via LED or manually
            if (hasGPIO)
                normalOperation();
            else
                cout << "Result: valid (G)" << endl;
            lastCommand = "RESEND"; // Update last command
        }
    }
    else if (command == "STATUS")
    {
        if (lastFormattedMessage.empty())
        {
            // No message to check
            json message = {
                {"command", "STATUS"},
                {"last_command", lastCommand.empty() ? "None" : lastCommand},
                {"timestamp", getCurrentTimestamp()},
                {"MAC Validation", false},
                {"status", "No data sent"}};
            cout << "System Status: " << message.dump(4) << endl;

            if (hasGPIO)
                errorDetected();
            else
                cout << "Result: invalid (R)" << endl;
            allOff();
        }
        else
        {
            // Attempt to decrypt and verify the last message
            vector<unsigned char> ciphertext = macAndEncrypt(lastFormattedMessage, key, iv, macKey);
            vector<unsigned char> data = decrypt(ciphertext, key, iv);

            // Generate keys
            //  Extract MAC,hash, and original JSON
            vector<unsigned char> extractedMAC(data.begin(), data.begin() + 32);       // First 32 bytes = MAC
            vector<unsigned char> extractedHash(data.begin() + 32, data.begin() + 64); // second 32 bytes = hash
            vector<unsigned char> extractedPlaintext(data.begin() + 64, data.end());   // Rest = cyphertext

            // Validate the MAC
            vector<unsigned char> macData(extractedHash.begin(), extractedHash.end());
            macData.insert(macData.end(), extractedPlaintext.begin(), extractedPlaintext.end());

            bool result = validateMAC(macData, macKey, extractedMAC);

            // Construct a simplified JSON output
            json parsedMessage = json::parse(lastFormattedMessage);
            json message = {
                {"command", parsedMessage["command"]},
                {"last_command", lastCommand},
                {"timestamp", parsedMessage["timestamp"]},
                {"MAC Validation", result},
                {"status", result ? "Normal" : "Tamper"}};

            cout << "System Status: " << message.dump(4) << endl;

            // Indicate status via LED
            if (result)
            {
                // Indicate error via LED or manually
                if (hasGPIO)
                    normalOperation();
                else
                    cout << "Result: valid (G)" << endl;
            }
            else
            {
                // Indicate error via LED or manually
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
            cerr << "No message to decrypt!" << endl;
            // Indicate error via LED
            if (hasGPIO)
                errorDetected();
            else
                cout << "Result: invalid (R)" << endl;
            this_thread::sleep_for(chrono::seconds(3));
            allOff();
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
                cout << "MAC Validation: True" << endl;
                // Indicate error via LED
                if (hasGPIO)
                    normalOperation();
                else
                    cout << "Result: valid (G)" << endl;
            }
            else if (result.plaintext.empty())
            {
                cerr << "Decryption failed: empty plaintext." << endl;
                if (hasGPIO)
                    errorDetected();
                else
                    cout << "Result: invalid (R)" << endl;
                return;
            }
            else
            {
                cout << "MAC Validation: False" << endl;
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
        cout << "Exiting program..." << endl;
        unexportGPIO(GREEN_LED);
        unexportGPIO(RED_LED);
        exit(0);
    }
    else
    {
        cerr << "Unknown command: " << command << endl;
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
            hasGPIO = checkGPIOAvailability();
            if (hasGPIO)
            {
                cout << "GPIO detected. Using GPIO LEDs." << endl;
                cout << "Green LED GPIO Pin: 19" << endl;
                cout << "Red LED GPIO Pin: 26" << endl;

                char setupResponse;
                cout << "Do you need 5 minutes to set up the breadboard? (y/n): ";
                cin >> setupResponse;
                if (setupResponse == 'y' || setupResponse == 'Y')
                {
                    // clear the input buffer before the timer starts
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    waitForSetup();
                }
                cout << "Proceeding to initialize GPIO setup..." << endl;
                // Initialize LEDs
                exportGPIO(GREEN_LED);
                exportGPIO(RED_LED);
                setDirection(GREEN_LED, " out");
                setDirection(RED_LED, " out");
            }
            else
            {
                cout << "GPIO not accessible. Falling back to text output." << endl;
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
            cout << "Invalid response, please enter 'y' or 'n'. " << endl;
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
        allOff();
        unexportGPIO(GREEN_LED);
        unexportGPIO(RED_LED);
    }

    return 0;
}
