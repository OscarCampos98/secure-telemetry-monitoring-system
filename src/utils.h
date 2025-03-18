#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include "thread"
#include <openssl/sha.h>  // Required for SHA-256 functions
#include <openssl/hmac.h> // Required for HMAC functions

using namespace std;

string getCurrentTimestamp();
string getHMACKey();
vector<unsigned char> generateHMAC(const vector<unsigned char> &data, const vector<unsigned char> &key);
string generateKeyFromSeed(const string &seed);

#endif // UTILS_H
