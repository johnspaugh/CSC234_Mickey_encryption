#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <queue>
#include <array>
#include <sstream>

#define RUBIX_SIZE 256
#define KEY_SIZE 1024
#define SHUFFLE_SIZE 16777216 // 16M

using namespace std;

// Function to load Pi binary digits from file
string loadPiDigits(const string &filename) {
    ifstream file(filename);
    string pi_digits;
    if (file) {
        getline(file, pi_digits); // Read entire line as binary string
        file.close();
    } else {
        cerr << "Error: Unable to load Pi digits from file!" << endl;
        exit(1);
    }
    return pi_digits;
}

// Pi Digits loaded dynamically
const string PI_DIGITS = loadPiDigits("pi_10000_binary.txt");

// Huffman Node structure
struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// Generate Huffman Codes
void generateHuffmanCodes(HuffmanNode* root, string code, unordered_map<char, string> &huffmanTable) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanTable[root->data] = code;
    }
    generateHuffmanCodes(root->left, code + "0", huffmanTable);
    generateHuffmanCodes(root->right, code + "1", huffmanTable);
}

// Process Key: Ensure 1KB length, pad with Pi digits
void processKey(string &key) {
    if (key.size() > KEY_SIZE) {
        key = key.substr(0, KEY_SIZE);
    } else {
        size_t piIndex = hash<string>{}(key) % PI_DIGITS.size(); // Hash-based starting point
        while (key.size() < KEY_SIZE) {
            key += PI_DIGITS[piIndex % PI_DIGITS.size()];
            piIndex++;
        }
    }
}

// XOR Operation
void xorOperation(vector<unsigned char> &data, const string &key) {
    for (size_t i = 0; i < data.size(); i++) {
        data[i] ^= key[i % KEY_SIZE];
    }
}

// Load and Save Files
vector<unsigned char> loadFile(const string &filename) {
    ifstream file(filename, ios::binary);
    return vector<unsigned char>((istreambuf_iterator<char>(file)), {});
}

void saveFile(const string &filename, const vector<unsigned char> &data) {
    ofstream file(filename, ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Encrypt File
void encryptFile(const string &inputFile, const string &outputFile, string key) {
    vector<unsigned char> data = loadFile(inputFile);
    processKey(key);
    xorOperation(data, key);
    unordered_map<char, string> huffmanTable;
    string encodedData = "";
    for (char c : data) encodedData += bitset<8>(c).to_string();
    saveFile(outputFile, vector<unsigned char>(encodedData.begin(), encodedData.end()));
}

// Decrypt File
void decryptFile(const string &inputFile, const string &outputFile, string key) {
    vector<unsigned char> data = loadFile(inputFile);
    processKey(key);
    string binaryString(data.begin(), data.end());
    string decodedData = "";
    for (size_t i = 0; i < binaryString.size(); i += 8) {
        bitset<8> byte(binaryString.substr(i, 8));
        decodedData += static_cast<char>(byte.to_ulong());
    }
    vector<unsigned char> decodedVector(decodedData.begin(), decodedData.end());
    xorOperation(decodedVector, key);
    saveFile(outputFile, decodedVector);
}

// Main Execution
int main() {
    string inputFile = "test.txt";
    string encryptedFile = "test.khn";
    string decryptedFile = "test_dec.txt";
    string key = "SuperSecretKey123456";

    cout << "Encrypting file..." << endl;
    encryptFile(inputFile, encryptedFile, key);
    cout << "Encryption complete: " << encryptedFile << endl;

    cout << "Decrypting file..." << endl;
    decryptFile(encryptedFile, decryptedFile, key);
    cout << "Decryption complete: " << decryptedFile << endl;

    return 0;
}