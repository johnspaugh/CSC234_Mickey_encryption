/*
* huffman.cpp
* @author Craig Beaubien
* 
* taken from https://www.geeksforgeeks.org/huffman-coding-in-cpp/
* contains some functions for creating the huffman coding
* 
* TODO: find the decoding functions
*       possibly better implementations, maybe one that isn't recursive, but we probably won't
*           have to recurse as much as initially anticipated
*/
#include "huffman.h"

/*
* this recursively generates the huffman codes
* 
* @param    root                pointer to a Node structure
* @param    code                current code
* @param    codes               map of a character to its corresponding huffman code
* 
* TODO: consider a non-recursive solution
* 
* @return   none
*/
static void generateCodes(Node* root, std::string code, std::map<unsigned char, std::string>& codes)
{
    if (root == nullptr) return;

    if (!root->left && !root->right)
        codes[root->ch] = code;

    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

/*
* Function to build the Huffman tree
* 
* @param    freqmap         map of frequencies of each character in the input file
* 
* TODO: figure out how this works to document it better.
* 
* @return   Node*           pointer to a Node structure
*/
Node* buildHuffmanTree(const std::array<uint32_t, 256>& freqMap)
{
    // Custom comparator for the priority queue
    auto Compare = [](Node* const& l, Node* const& r) {return l->freq > r->freq; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(Compare)> pq(Compare);

    // Create leaf nodes for each character
    for (size_t i = 0; i < 256; i++)
        pq.push(new Node((char)i, freqMap[i]));

    // Build the tree
    while (pq.size() > 1)
    {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* internalNode = new Node('\0', left->freq + right->freq);
        internalNode->left = left;
        internalNode->right = right;
        pq.push(internalNode);
    }

    return pq.top();
}

bool huffmanEncode(std::vector<uint8_t>& input, std::array<uint32_t, 256>& freq, std::vector<uint8_t>& encodedBytes, uint32_t &stringLength)
{
    for (auto i : input)
        freq[i]++;

    Node* root = buildHuffmanTree(freq);

    std::map<unsigned char, std::string> codes;
    generateCodes(root, "", codes);

    std::string encodedString;
    for (auto c : input)
        encodedString += codes[c];

    stringLength = static_cast<uint32_t>(encodedString.length());
    
    /*
     * we need to account for strings that don't end on byte boundry,
     * so let's pad it with '0's
     */
    encodedString.resize(stringLength + (8 - (stringLength % 8)), '0');

    for (int i = 0; i < encodedString.size(); i += 8)
        encodedBytes.push_back(uint8_t(std::bitset<8>(encodedString.substr(i, 8)).to_ulong()));

    return true;
}

// Function to decode a given Huffman encoded string
bool huffmanDecode(std::string &input, std::array<uint32_t, 256>& freq, std::vector<uint8_t>& decodedBytes)
{
    Node* root = buildHuffmanTree(freq);

    std::map<unsigned char, std::string> codes;
    generateCodes(root, "", codes);

    Node* curr = root;
    for (char bit : input) 
    {
        if (bit == '0')
        {
            if ((curr != nullptr) && (curr->left != nullptr))
               curr = curr->left;
        }
        else
        {
            if ((curr != nullptr) && (curr->right != nullptr))
                curr = curr->right;
        }
        // Reached a leaf node
        if ((curr != nullptr)
            && (curr->left == nullptr)
            && (curr->right == nullptr)
        ) 
        {
            //cout << curr->ch;
            decodedBytes.push_back(curr->ch);
            curr = root;
        }
    }

    return true;
}