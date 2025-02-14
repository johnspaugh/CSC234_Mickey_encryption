#pragma once
#include "file_encryptor.h"

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