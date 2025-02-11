/*
* huffman.h
* @author Craig Beaubien
* 
* taken from https://www.geeksforgeeks.org/huffman-coding-in-cpp/
* contains some functions for creating the huffman coding
* 
* TODO: find the decoding functions
*       possibly better implementations, maybe one that isn't recursive, but we probably won't
*           have to recurse as much as initially anticipated
*       lookup how to call these functions, might be buried in the inactive preprocessor block in main
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

    if (root->ch != '\0') 
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
Node* buildHuffmanTree(const std::array<int, 256>& freqMap)
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
