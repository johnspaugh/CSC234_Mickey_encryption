// file_encryptor.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "file_encryptor.h"

/*
 * This function gets the key from the specified file passed in. We create an fstream with the
 * name of the input file; we read everything in as binary. If the file can't be opened, return
 * false. After we read in the file, we verify it's at least 64 bytes. We then resize the buffer
 * to 1000 bytes, truncating the key if it's too long. If it's too short, we pad it with 'random'
 * data from the PI constant defined in the header.
 *
 * TODO: adjust the read in to buffer passed in
 *
 * @param   input_file              name of the file to read in
 * @param   key                     buffer to hold the key
 * @return  boolean
*/
bool get_key(std::string input_file, std::vector<uint8_t>& key)
{
    std::ifstream input(input_file, std::ios::binary);

    if (!input.is_open())
    {
        std::cerr << "Can't find key file: " << input_file << std::endl;
        return false;
    }

    // copies all data into buffer
    std::vector<uint8_t> key_buffer(std::istreambuf_iterator<char>(input), {});
    std::cout << "tellg: " << input.tellg() << std::endl;

    input.close();

    size_t key_size = key_buffer.size();
    if (key_size < 64)
    {
        std::cerr << "Invalid key." << std::endl;
        return false;
    }

    key_buffer.resize(MAX_KEY_SIZE);
    if (key_size < MAX_KEY_SIZE)
    {
        uint8_t start = ((key_buffer[HIGH_BYTE % key_size] * 10) + key_buffer[LOW_BYTE] % key_size);

        std::copy(PI + start, PI + start + (MAX_KEY_SIZE - key_size), key_buffer.begin() + key_size);
    }

    return true;
}

/*
 * This function parses all the command line options and stores them into a map passed in.
 * Right now, it only checks for the key flag and the file flag.
 * 
 * @param   argc                    number of command line parameters directly from main
 * @param   argv                    the command line parameters directly from main
 * @param   command_line_options    this is the map, passed in by reference to populate
 * @return  boolean                 if we have any issue parsing the command line options
 *                                      return false, otherwise return true;
*/
bool parse_options(int argc, char** argv, std::map<std::string, std::string>& command_line_options)
{
    for (int i = 1; i < argc; i++)
    {
        if ((std::strcmp(argv[i], "-k") == 0) && (i + 1 < argc))
            command_line_options["key_file"] = argv[i + 1];
        else if ((std::strcmp(argv[i], "-f") == 0) && (i + 1 < argc))
            command_line_options["encrypt_file"] = argv[i + 1];
    }

    return true;
}

/*
 * This function verifies the file and reads into the buffer provided from main. First we
 * create a stream and open the file at the same time, putting the position to the end of
 * the file so we can get the size. If the size is greater than 12 MB, we declare it too
 * big and return false. Then we return the position back to the beginning and resize the
 * input file buffer vector to the correct size to read in. Afterwards, we write 3 bytes
 * for the size of the file, then the file name. The file size is 3 bytes so 12 MB will fit.
 * The file name is written as characters to the buffer as well.
 * 
 * TODO: pad file to 16 MB to fill our 'rubix' array later.
 * 
 * @param   input_file              name of the file to read in
 * @param   input_file_buffer       buffer to store the file.
 * @return  boolean                 if we have any issue reading the file return false, 
 *                                      otherwise return true;
*/
bool read_file(std::string input_file, std::vector<uint8_t>& input_file_buffer)
{
    std::ifstream input(input_file, std::ifstream::ate | std::ios::binary);

    if (!input.is_open())
    {
        std::cerr << "Can't find input file: " << input_file << std::endl;
        return false;
    }

    std::ifstream::pos_type file_size = input.tellg();
    if (file_size > MAX_FILE_SIZE)
    {
        std::cerr << "File too big." << std::endl;
        input.close();
        return false;
    }

    input.seekg(0);

    input_file_buffer.resize(file_size);
    input.read(reinterpret_cast<char*>(input_file_buffer.data()), static_cast<int>(file_size));
    
    input.close();

    // write 3 bytes for the size of the file.
    for (int i = 2; i >= 0; i--)
        input_file_buffer.insert(input_file_buffer.begin() + (2-i), (file_size >> (i * 8)) & 0xff);

    input_file_buffer.insert(input_file_buffer.begin() + 3, (uint8_t)input_file.size());

    // write the file name to the end of the buffer
    input_file_buffer.insert(input_file_buffer.begin() + 4, input_file.begin(), input_file.end());

    return true;
}

/*
 * This function rotates a 'row' in the 'x' direction an amount specified. We uses
 * std::rotate as it has the wrap function specified.
 *
 * TODO: implement using std::rotate
 *
 * @param   amount              name of the file to read in
 * @return  void
*/
void shift_x(uint8_t amount)
{

}

/*
 * This function rotates a 'column' in the 'y' direction an amount specified. We uses
 * std::rotate as it has the wrap function specified.
 *
 * TODO: implement using std::rotate. we'll probably have to write the 'column' to a temporary
 * buffer, rotate it, then write it back. I think it'll cost the same as trying to rotate in place.
 *
 * @param   amount              name of the file to read in
 * @return  void
*/
void shift_y(char amount) 
{

}


/*
 * This function rotates a 'drawer' in the 'z' direction an amount specified. We uses
 * std::rotate as it has the wrap function specified.
 *
 * @TODO: implement using std::rotate. we'll probably have to write the 'drawer' to a temporary
 * buffer, rotate it, then write it back. I think it'll cost the same as trying to rotate in place.
 *
 * @param   amount              name of the file to read in
 * @return  void
*/
void shift_z(char amount)
{

}

/*
 * This function main entry point of the program. Used mainly as driver to parse the command line
 * options, get the encyption key, and read in the file to encrypt. Will print to standard error if
 * we encounter a problem with any of the parsing/reading so far.
 *
 * TODO: huffman functions taken from web in another file, need to incorporate to this one before doing
 * the rubix shuffle. Also need to figure out how to write the rubix array to the output file.
 *
 * @param   argc                    number of command line parameters
 * @param   argv                    command line parameters
 * @return  int                     0 if successful, -1 otherwise
*/

int main(int argc, char **argv)
{
    std::map<std::string, std::string> command_line_options;

    if (parse_options(argc, argv, command_line_options) == false)
    {
        std::cout << "Invalid options" << std::endl;
        exit(-1);
    }

    std::vector<uint8_t> key;
    if (get_key(command_line_options["key_file"], key) == false)
    {
        std::cerr << "Error with key." << std::endl;
        exit(-1);
    }

    std::vector<uint8_t> file_buffer;
    if (read_file(command_line_options["encrypt_file"], file_buffer) == false)
    {
        std::cerr << "Error with input file." << std::endl;
        exit(-1);
    }

    for (int i = 0; i < file_buffer.size(); i++)
        file_buffer[i] ^= key[i % MAX_KEY_SIZE];

    //Build Huffman tree and generate codes
    Node* root = buildHuffmanTree(byte_array);
    std::map<unsigned char, std::string> codes;
    generateCodes(root, "", codes);

    //// Print Huffman Codes
    //std::cout << "Huffman Codes:\n";
    //for (auto pair : codes) {
    //    std::cout << pair.first << " " << pair.second << "\n";
    //}

    //// Reinterpret the array with different indices
    //uint8_t(*p)[3][3][3] = reinterpret_cast<uint8_t(*)[3][3][3]>(array.data());

    //for (int i = 0; i < 3; i++)
    //    for (int j = 0; j < 3; j++)
    //        std::rotate(std::begin((*p)[i][j]), std::begin((*p)[i][j]) + 2, std::end((*p)[i][j]));


    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
