#pragma once
#include "file_encryptor.h"
#include <array>
#include <bitset>

// Structure to represent a node in the Huffman tree
struct Node
{
    char ch;
    int freq;
    Node* left, * right;

    Node(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
};

void generateCodes(Node* root, std::string code, std::map<unsigned char, std::string>& codes);
Node* buildHuffmanTree(const std::array<int, 256>& freqMap);
bool huffmanEncode(std::vector<uint8_t>& input, std::array<uint32_t, 256>& freq, std::vector<uint8_t>& encodedBytes, uint32_t &stringLength);
bool huffmanDecode(std::string& input, std::array<uint32_t, 256>& freq, std::vector<uint8_t>& decodedBytes);
