
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

// Constants: First 10,000 digits of Pi in binary
const string PI_DIGITS = "121314..........................................";

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

void generateHuffmanCodes(HuffmanNode* root, string code, unordered_map<char, string> &huffmanTable) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanTable[root->data] = code;
    }
    generateHuffmanCodes(root->left, code + "0", huffmanTable);
    generateHuffmanCodes(root->right, code + "1", huffmanTable);
}

string huffmanEncode(const string &data, unordered_map<char, string> &huffmanTable) {
    unordered_map<char, int> freq;
    for (char c : data) freq[c]++;
    
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
    for (auto pair : freq) pq.push(new HuffmanNode(pair.first, pair.second));
    
    while (pq.size() > 1) {
        HuffmanNode *left = pq.top(); pq.pop();
        HuffmanNode *right = pq.top(); pq.pop();
        HuffmanNode *merged = new HuffmanNode('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        pq.push(merged);
    }
    generateHuffmanCodes(pq.top(), "", huffmanTable);
    
    string encodedString = "";
    for (char c : data) encodedString += huffmanTable[c];
    return encodedString;
}

// Huffman Decoding
string huffmanDecode(const string &encodedData, HuffmanNode* root) {
    string decodedString = "";
    HuffmanNode* curr = root;
    for (char bit : encodedData) {
        curr = (bit == '0') ? curr->left : curr->right;
        if (!curr->left && !curr->right) {
            decodedString += curr->data;
            curr = root;
        }
    }
    return decodedString;
}

void processKey(string &key) {
    if (key.size() > KEY_SIZE) {
        key = key.substr(0, KEY_SIZE);
    } else {
        while (key.size() < KEY_SIZE) {
            key += PI_DIGITS[key.size() % PI_DIGITS.size()];
        }
    }
}

void xorOperation(vector<unsigned char> &data, const string &key) {
    for (size_t i = 0; i < data.size(); i++) {
        data[i] ^= key[i % KEY_SIZE];
    }
}

vector<unsigned char> loadFile(const string &filename) {
    ifstream file(filename, ios::binary);
    return vector<unsigned char>((istreambuf_iterator<char>(file)), {});
}

void saveFile(const string &filename, const vector<unsigned char> &data) {
    ofstream file(filename, ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void encryptFile(const string &inputFile, const string &outputFile, string key) {
    vector<unsigned char> data = loadFile(inputFile);
    processKey(key);
    xorOperation(data, key);
    unordered_map<char, string> huffmanTable;
    string encodedData = huffmanEncode(string(data.begin(), data.end()), huffmanTable);
    saveFile(outputFile, vector<unsigned char>(encodedData.begin(), encodedData.end()));
}

void decryptFile(const string &inputFile, const string &outputFile, string key) {
    vector<unsigned char> data = loadFile(inputFile);
    processKey(key);
    unordered_map<char, string> huffmanTable;
    HuffmanNode* huffmanTree = nullptr; // Placeholder: Tree should be reconstructed
    string decodedData = huffmanDecode(string(data.begin(), data.end()), huffmanTree);
    vector<unsigned char> decodedVector(decodedData.begin(), decodedData.end());
    xorOperation(decodedVector, key);
    saveFile(outputFile, decodedVector);
}

int main() {
    string inputFile = "test.txt";
    string encryptedFile = "test.khn";
    string decryptedFile = "test_dec.txt";
    string key = "SuperSecretKey123";

    cout << "Encrypting file..." << endl;
    encryptFile(inputFile, encryptedFile, key);
    cout << "Encryption complete: " << encryptedFile << endl;

    cout << "Decrypting file..." << endl;
    decryptFile(encryptedFile, decryptedFile, key);
    cout << "Decryption complete: " << decryptedFile << endl;

    return 0;
}
