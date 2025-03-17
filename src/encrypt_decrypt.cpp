#include "telemetry.h" // Include telemetry functions
#include "utils.h"     // Include utility functions
#include "encrypt_decrypt.h"

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "json.hpp"       // JSON library
#include <iomanip>        // For hex formatting
#include <openssl/evp.h>  // OpenSSL EVP API
#include <openssl/rand.h> // For generating random IV
#include <openssl/hmac.h> // HMAC functions

using namespace std;
using json = nlohmann::json;
// Helper function for debugging

void printHex(const string &label, const vector<unsigned char> &data)
{
    cout << label << ": ";
    for (unsigned char byte : data)
    {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(byte);
    }
    cout << endl;
}

// Helper functions:

// format the input into the desired structure
string formatString(const string &command)
{
    // Generate telemetry data
    json telemetryData = generateTelemetryData();

    // Get the current timestamp
    string timestamp = getCurrentTimestamp();

    // Construct the message with command, telemetry, and timestamp
    json message = json{
        {"command", command},         // Command (e.g., SEND, RESEND)
        {"telemetry", telemetryData}, // Telemetry data
        {"timestamp", timestamp}      // Timestamp of the message creation

    };

    return message.dump(); // Return the structured message as JSON string
}

// Generate a random 256-bit key
vector<unsigned char> generateRandomKey(int length)
{
    vector<unsigned char> key(length);

    if (!RAND_bytes(key.data(), length))
    {
        throw runtime_error("Failed to generate random key");
    }

    return key;
}

// Generate a random 128-bit IV
vector<unsigned char> generateRandomIV(int length)
{
    vector<unsigned char> iv(length);   // allocate space
    if (!RAND_bytes(iv.data(), length)) // Fill with random bytes

    {
        throw runtime_error("Failed to generate random IV");
    }
    return iv;
}

// Convert Binary data to a  hexadecimal format for display
string toHexString(const vector<unsigned char> &hash)
{

    ostringstream oss;
    for (unsigned char byte : hash)
    {
        oss << hex << setw(2) << setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

// function to compute sha-256 hash
vector<unsigned char> computeSHA256(const string &input)
{
    // Context for the digest operation
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        throw runtime_error("Failed to create EVP_MD_CTX");
    }

    // Initialize the context with the SHA-256 algorithm
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Failed to initialize digest");
    }

    // Update the context with the input data
    if (EVP_DigestUpdate(ctx, input.data(), input.size()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Failed to update digest");
    }

    // Finalize the digest and retrieve the hash
    vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
    unsigned int hashLength = 0;
    if (EVP_DigestFinal_ex(ctx, hash.data(), &hashLength) != 1)
    {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(ctx); // Free the context
    return hash;
}

// Encrypt the hashed data using AES-256-CBC
vector<unsigned char> encrypt(const vector<unsigned char> &plaintext,
                              const vector<unsigned char> &key, vector<unsigned char> &iv)
{

    // create a new encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        throw runtime_error("Failed to create EVP_CIPHER_CTX ");
    }

    // Initialize the encryption operation with AES-256-CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx); // Clean up failure
        throw runtime_error("Failed to init encryption");
    }

    // Prepare buffer for ciphertext(plaintext size + block size for padding)
    vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, ciphertext_len = 0;

    // Encrypt the plaintext
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx); // clean up failure
        throw runtime_error("Failed to encrypt data");
    }

    ciphertext_len = len;

    // Finalize encryption (handle any remaining padding)
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx); // clean up failure
        throw runtime_error("Failed to finalize encrypt");
    }

    ciphertext_len += len;             // Add the final block's length
    ciphertext.resize(ciphertext_len); // resize the actual ciphertext;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

/** Moved "generateHMAC" function to utils.cpp
 * Generate an HMAC using SHA-256
 */
//

vector<unsigned char> macAndEncrypt(const string &message, const vector<unsigned char> &key,
                                    vector<unsigned char> &iv, const vector<unsigned char> &macKey)
{
    // Generate the SHA-256 hash of the message
    vector<unsigned char> hash = computeSHA256(message);

    // Combine the hash and the JSON message
    vector<unsigned char> plaintext(hash.begin(), hash.end());
    plaintext.insert(plaintext.end(), message.begin(), message.end());

    // Generate the MAC for the plaintext
    vector<unsigned char> mac = generateHMAC(plaintext, macKey);
    // prepend the MAC for the plaintext
    plaintext.insert(plaintext.begin(), mac.begin(), mac.end());

    // Encrypt the combine data
    return encrypt(plaintext, key, iv);
}

// Decrypt the ciphertext using AES-256-CBC
vector<unsigned char> decrypt(const vector<unsigned char> &ciphertext, const vector<unsigned char> &key,
                              const vector<unsigned char> &iv)
{

    // Create a decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        throw runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    try
    {
        // Initialize the decryption operation
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        {
            throw runtime_error("Failed to initialize decryption");
        }

        // Prepare buffer for plaintext
        vector<unsigned char> plaintext(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        int len = 0, plaintext_len = 0;

        // Decrypt the ciphertext
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1)
        {
            throw runtime_error("Failed to decrypt data during update.");
        }
        plaintext_len += len;

        // Finalize the decryption
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len, &len) != 1)
        {
            throw runtime_error("Failed to finalize decryption: Check padding or ciphertext integrity.");
        }
        plaintext_len += len;

        // Resize to actual plaintext length
        plaintext.resize(plaintext_len);
        EVP_CIPHER_CTX_free(ctx);
        return plaintext;
    }
    catch (const exception &e)
    {
        EVP_CIPHER_CTX_free(ctx);
        cerr << "Error during decryption: " << e.what() << endl;
        throw; // Rethrow exception after cleanup
    }
}

// Validate the MAC
bool validateMAC(const vector<unsigned char> &plaintext, const vector<unsigned char> &macKey,
                 const vector<unsigned char> &originalMAC)
{

    // compute the HMAC for the plaintext
    vector<unsigned char> computedMAC = generateHMAC(plaintext, macKey);

    // Compare original MAC with computed MAC
    if (computedMAC == originalMAC)
    {
        return true;
    }
    else
    {
        cerr << "MAC validation failed: Computed MAC does not match the original MAC." << endl;
        return false;
    }
}

// Decrypt and verify
DecryptionResult decryptAndVerify(const std::vector<unsigned char> &ciphertext,
                                  const std::vector<unsigned char> &key,
                                  const std::vector<unsigned char> &iv,
                                  const std::vector<unsigned char> &macKey)
{

    try
    {
        if (ciphertext.empty())
        {
            throw runtime_error("Ciphertext is empty.");
        }
        if (key.size() != 32 || iv.size() != 16)
        {
            throw runtime_error("Invalid key or IV size.");
        }

        // Decrypt the ciphertext
        vector<unsigned char> decryptedData = decrypt(ciphertext, key, iv);

        // Log decrypted data size in logger.cpp
        // cerr << "Decrypted data size: " << decryptedData.size() << " bytes" << endl;

        // Validate size before extracting components
        if (decryptedData.size() < 64)
        {
            throw runtime_error("Decrypted data too short to extract components (requires at least 64 bytes). ");
        }

        // Extract MAC,hash, and original JSON
        vector<unsigned char> extractedMAC(decryptedData.begin(), decryptedData.begin() + 32);       // First 32 bytes = MAC
        vector<unsigned char> extractedHash(decryptedData.begin() + 32, decryptedData.begin() + 64); // second 32 bytes = hash
        vector<unsigned char> extractedPlaintext(decryptedData.begin() + 64, decryptedData.end());   // Rest = cyphertext

        // Convert plaintext to string
        string plaintext;
        try
        {
            plaintext = string(extractedPlaintext.begin(), extractedPlaintext.end());
        }
        catch (const exception &e)
        {
            throw runtime_error("Failed to convert plaintext to string: " + string(e.what()));
        }

        // Log extracted components in logger.cpp
        // printHex("Extracted MAC", extractedMAC);
        // printHex("Extracted Hash", extractedHash);
        // cerr << "Extracted Plaintext Size: " << plaintext.size() << " bytes" << endl;
        // cerr << "Extracted Plaintext: " << plaintext << endl;

        // Validate the MAC
        vector<unsigned char> macData(extractedHash.begin(), extractedHash.end());
        macData.insert(macData.end(), extractedPlaintext.begin(), extractedPlaintext.end());

        if (!validateMAC(macData, macKey, extractedMAC))
        {
            throw runtime_error("HMAC validation failed: Message integrity compromised.");
        }

        // Log success and return result
        string decryptionTimestamp = getCurrentTimestamp();
        // cerr << "Decryption successful, Timestamp: " << decryptionTimestamp << endl;

        return {std::move(plaintext), true, decryptionTimestamp, extractedHash};
    }
    catch (const exception &e)
    {
        cerr << "Error during decryption and verification: " << e.what() << endl;
        return {"", false, getCurrentTimestamp(), {}}; // Return empty plaintext and invalid MAC on failure
    }
}
/*
int main()
{
    try
    {
        // Example command
        string command = "START";

        // Step 1: Format the input string
        string formatted = formatString(command);

        // Step 2: Compute SHA-256 hash of the formatted string
        vector<unsigned char> hash = computeSHA256(formatted);

        // Step 3: Generate random encryption key, IV, and MAC key
        vector<unsigned char> key = generateRandomKey(32);    // 256-bit key for encryption
        vector<unsigned char> iv = generateRandomIV(16);      // 128-bit IV
        vector<unsigned char> macKey = generateRandomKey(32); // 256-bit key for HMAC

        // Step 4: Encrypt the data using MAC and encryption
        vector<unsigned char> ciphertext = macAndEncrypt(formatted, key, iv, macKey);

        // Step 5: Decrypt and verify the encrypted data (retrieve hash and verify integrity)
        DecryptionResult result = decryptAndVerify(ciphertext, key, iv, macKey);

        // Step 6: Output the results
        cout << "=== Output Results ===:" << endl;
        cout << "\nOriginal Command: " << command << endl;
        cout << "Formatted JSON: " << formatted << endl;
        cout << "SHA-256 Hash (Hex): " << toHexString(hash) << endl;
        cout << "Encryption Key (Hex): " << toHexString(key) << endl;
        cout << "Initialization Vector (IV): " << toHexString(iv) << endl;
        cout << "MAC Key (Hex): " << toHexString(macKey) << endl;
        cout << "Ciphertext (Hex): " << toHexString(ciphertext) << endl;

        if (result.macValid)
        {
            cout << "Decrypted Plaintext: " << result.plaintext << endl;
            cout << "Decryption Timestamp: " << result.timestamp << endl;
            cout << "Extracted Hash (Hex): " << toHexString(result.hash) << endl;
            cout << "MAC Validation: PASS" << endl;
        }
        else
        {
            cerr << "Decryption failed. MAC Validation: FAIL" << endl;
        }

        // Step 7: Simulate tampering and test error handling
        cout << "\n=== Testing Tampering ===" << endl;
        ciphertext[10] ^= 0xFF; // Flip a random bit in the ciphertext
        try
        {
            DecryptionResult tamperedResult = decryptAndVerify(ciphertext, key, iv, macKey);
            if (!tamperedResult.macValid)
            {
                cout << "Tampered ciphertext detected. MAC Validation: FAIL" << endl;
            }
        }
        catch (const runtime_error &e)
        {
            cerr << "Tampering Detected: " << e.what() << endl;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
*/
