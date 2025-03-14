#ifndef ENCRYPT_DECRYPT_H
#define ENCRYPT_DECRYPT_H

#include <string>
#include <vector>

// Struct to encapsulate decryption results
struct DecryptionResult
{
    std::string plaintext;           // Decrypted JSON as a string
    bool macValid;                   // MAC Validation
    std::string timestamp;           // Timestamp of decryption (for logs)
    std::vector<unsigned char> hash; // Extracted hash (for forensic analysis)
};

// Function prototypes
std::string formatString(const std::string &command);
std::vector<unsigned char> generateRandomKey(int length);
std::vector<unsigned char> generateRandomIV(int length);
std::string toHexString(const std::vector<unsigned char> &data);
std::vector<unsigned char> computeSHA256(const std::string &data);
std::vector<unsigned char> encrypt(const std::vector<unsigned char> &plaintext, const std::vector<unsigned char> &key, std::vector<unsigned char> &iv);
std::vector<unsigned char> generateHMAC(const std::vector<unsigned char> &data, const std::vector<unsigned char> &key);
std::vector<unsigned char> macAndEncrypt(const std::string &message, const std::vector<unsigned char> &key, std::vector<unsigned char> &iv, const std::vector<unsigned char> &macKey);
std::vector<unsigned char> decrypt(const std::vector<unsigned char> &ciphertext, const std::vector<unsigned char> &key, const std::vector<unsigned char> &iv);
bool validateMAC(const std::vector<unsigned char> &mac, const std::vector<unsigned char> &hash, const std::vector<unsigned char> &macKey);
DecryptionResult decryptAndVerify(const vector<unsigned char> &ciphertext, const vector<unsigned char> &key, const vector<unsigned char> &iv, const vector<unsigned char> &macKey);

#endif // ENCRYPT_DECRYPT_H
