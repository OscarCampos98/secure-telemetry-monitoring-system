#include "utils.h" // Include the header file for the utility functions

#include <random>    // For random number generation
#include <string>    // For handling strings
#include <fstream>   // For file input/output
#include <stdexcept> // For runtime_error
#include <iostream>  // For console input and output
#include <iomanip>   // For formatting output (e.g., timestamp)
#include <chrono>    // For generating timestamps
#include "thread"
#include <openssl/sha.h>  // Required for SHA256 functions
#include <openssl/hmac.h> // Required for HMAC functions

using namespace std;

// Function to generate the current timestamp in ISO 8601 format
string getCurrentTimestamp()
{
    // Get the current system time
    auto now = chrono::system_clock::now();

    // Convert system time to a time_t object for formatting
    auto now_time = chrono::system_clock::to_time_t(now);

    // Convert time_t to a UTC time structure
    tm utc_time = *gmtime(&now_time);

    // Format the time into an ISO 8601 string
    ostringstream oss;
    oss << put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ"); // e.g., "2024-12-05T14:23:05Z"
    return oss.str();                                 // Return the formatted timestamp as a string
}

// Function to generate a 256-bit (32 byte) HMAC key from seed
string generateKeyFromSeed(const string &seed)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];                                         // Buffer for the hash
    SHA256(reinterpret_cast<const unsigned char *>(seed.c_str()), seed.size(), hash); // Compute the SHA-256 hash

    ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        oss << hex << setw(2) << setfill('0') << (int)hash[i]; // Convert each byte to a 2-digit hex value
    }
    return oss.str(); // Return the 64-character hex string
}
// Function to read the HMAC key from a configuration file
string getHMACKey()
{
    ifstream file("/etc/hmac_key.conf");
    string key;
    if (file)
    {
        getline(file, key);
        file.close();

        // Ensure the key is a valid hex string
        if (key.length() != 64) // HMAC key should be 32 bytes = 64 hex chars
        {
            cerr << "ERROR: Invalid HMAC key length!" << endl;
            throw runtime_error("Invalid HMAC key length in file.");
        }

        for (char c : key)
        {
            if (!isxdigit(c))
            {
                cerr << "ERROR: Non-hex character found in HMAC key!" << endl;
                throw runtime_error("Invalid HMAC key format.");
            }
        }

        cout << "DEBUG: Loaded HMAC key: " << key << endl;
        return key;
    }
    else
    {
        cerr << "ERROR: Unable to open /etc/hmac_key.conf. Check file permissions." << endl;
        throw runtime_error("Failed to open HMAC key file!");
    }
}

// Function to generate HMAC using SHA-256
vector<unsigned char> generateHMAC(const vector<unsigned char> &data, const vector<unsigned char> &key)
{

    // buffer for HMAC
    unsigned int macLength = 0;
    vector<unsigned char> mac(EVP_MAX_MD_SIZE);

    // compute the HMAC using openSSL
    unsigned char *result = HMAC(EVP_sha256(), key.data(), key.size(), data.data(), data.size(), mac.data(), &macLength);
    if (!result)
    {
        throw runtime_error("Failed to compute MAC");
    }

    // Resize to the actual MAC length
    mac.resize(macLength);
    return mac;
}

vector<unsigned char> hexStringToBytes(const string &hex)
{
    vector<unsigned char> bytes;

    if (hex.length() % 2 != 0)
    {
        cerr << "ERROR: Invalid hex string length!" << endl;
        throw invalid_argument("Hex string must have an even number of characters.");
    }

    for (size_t i = 0; i < hex.length(); i += 2)
    {
        string byteString = hex.substr(i, 2);

        if (!isxdigit(byteString[0]) || !isxdigit(byteString[1]))
        {
            cerr << "ERROR: Invalid hex digit found in key!" << endl;
            throw invalid_argument("Invalid character in hex string.");
        }

        bytes.push_back(static_cast<unsigned char>(stoul(byteString, nullptr, 16)));
    }

    return bytes;
}
