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
bool get_key(std::string input_file, std::vector<uint8_t>& key_file_buffer)
{
    std::ifstream input(input_file, std::ios::ate | std::ios::binary);

    if (!input.is_open())
    {
        std::cerr << "Can't find key file: " << input_file << std::endl;
        return false;
    }

    std::ifstream::pos_type key_size = input.tellg();
    if (key_size < 64)
    {
        std::cerr << "Invalid key." << std::endl;
        input.close();
        return false;
    }

    input.seekg(0);

    key_file_buffer.resize(key_size);
    input.read(reinterpret_cast<char*>(key_file_buffer.data()), static_cast<int>(key_size));
    input.close();


    key_file_buffer.resize(MAX_KEY_SIZE);
    if (key_size < MAX_KEY_SIZE)
    {
        uint8_t start = ((key_file_buffer[HIGH_BYTE % key_size] * 10) + key_file_buffer[LOW_BYTE] % key_size);
        std::copy(PI + start, PI + start + (MAX_KEY_SIZE - key_size), key_file_buffer.begin() + key_size);
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
        else if (std::strcmp(argv[i], "decode") == 0)
            command_line_options["decode"] = "true";
    }

    if (command_line_options.find("decode") == command_line_options.end())
        command_line_options["decode"] = "false";

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
bool read_file(std::string input_file, std::vector<file_buffer_type>& input_file_buffer)
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
#if BRUTE_FORCE
    input_file_buffer.resize(file_size);
    input.read(reinterpret_cast<char*>(input_file_buffer.data()), static_cast<int>(file_size));
#else
    std::vector<uint8_t> temp(file_size);
    input.read(reinterpret_cast<char*>(temp.data()), static_cast<int>(file_size));
#endif
    input.close();

#if 0
    // write 3 bytes for the size of the file.
    for (int i = 2; i >= 0; i--)
        temp_buffer.insert(temp_buffer.begin() + (2-i), (file_size >> (i * 8)) & 0xff);

    temp_buffer.insert(temp_buffer.begin() + 3, (uint8_t)input_file.size());

    // write the file name to the end of the buffer
    temp_buffer.insert(temp_buffer.begin() + 4, input_file.begin(), input_file.end());

    //TODO: fill remaining array with random noise
#endif

#if BRUTE_FORCE
    input_file_buffer.resize(RUBIX_SIDE_SIZE * RUBIX_SIDE_SIZE * RUBIX_SIDE_SIZE, '0');
#else
    std::copy(temp.begin(), temp.end(), std::back_inserter(input_file_buffer));
    input_file_buffer.erase(input_file_buffer.begin());

    // Reinterpret the array with different indices
    uint32_t(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<uint32_t(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input_file_buffer.data());

    for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                (*p)[z][y][x] |= (x << X_OFFSET);
                (*p)[z][y][x] |= (y << Y_OFFSET);
                (*p)[z][y][x] |= (z << Z_OFFSET);
            }

#endif
    return true;
}

#if BRUTE_FORCE
/*
 * This function rotates a 'row' in the 'x' direction an amount specified. We use
 * std::rotate as it has the wrap function specified.
 *
 * @param   input               buffer to file to encrypt
 * @param   key                 key dictating how much to shift each row
 * @return  void
*/
void shift_x(std::vector<file_buffer_type>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    file_buffer_type(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<file_buffer_type(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    uint8_t key_index = 0;
    for (int x = 0; x < RUBIX_SIDE_SIZE; x++)
        for (int y = 0; y < RUBIX_SIDE_SIZE; y++)
            if (decode)
                std::rotate(std::begin((*p)[x][y]), std::begin((*p)[x][y]) + key[y], std::end((*p)[x][y]));
            else
                std::rotate(std::rbegin((*p)[x][y]), std::rbegin((*p)[x][y]) + key[y], std::rend((*p)[x][y]));
}

/*
 * This function rotates a 'column' in the 'y' direction an amount specified. We use
 * std::rotate as it has the wrap function specified. I don't know if there's a better
 * method than removing the column, rotating it, then putting it back.
 *
 * @param   input               file buffer to decrypt
 * @param   key                 key dictating how much to shift each column
 * @return  void
*/
void shift_y(std::vector<file_buffer_type>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    file_buffer_type(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<file_buffer_type(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
    {
        uint8_t key_index = 0;
        for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        {
            std::vector<file_buffer_type> temp(RUBIX_SIDE_SIZE);
            for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
                temp[y] = (*p)[x][y][z];

            if (decode)
                std::rotate(temp.begin(), temp.begin() + key[key_index], temp.end());
            else
                std::rotate(temp.rbegin(), temp.rbegin() + key[key_index], temp.rend());

            for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
                (*p)[x][y][z] = temp[y];
            key_index = (key_index + 1) % RUBIX_SIDE_SIZE;
        }
    }
}

/*
 * This function rotates a 'drawer' in the 'z' direction an amount specified. We use
 * std::rotate as it has the wrap function specified.
 *
 * @TODO: implement using std::rotate. we'll probably have to write the 'drawer' to a temporary
 * buffer, rotate it, then write it back. I think it'll cost the same as trying to rotate in place.
 *
 * @param   input               file buffer to decrypt
 * @param   key                 key dictating how much to shift each column
 * @return  void
*/
void shift_z(std::vector<file_buffer_type>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    file_buffer_type(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<file_buffer_type(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
    {
        uint8_t key_index = 0;
        for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
        {
            std::vector<file_buffer_type> temp(RUBIX_SIDE_SIZE);
            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
                temp[x] = (*p)[x][y][z];

            if (decode)
                std::rotate(temp.begin(), temp.begin() + key[key_index], temp.end());
            else
                std::rotate(temp.rbegin(), temp.rbegin() + key[z], temp.rend());

            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
                (*p)[x][y][z] = temp[x];
            key_index = (key_index + 1) % RUBIX_SIDE_SIZE;
        }
    }
}
#endif      //BRUTE_FORCE

/*
 * This function writes the encripted file to disk. Since ofstream::write takes
 * char *, we can cast it either way BRUTE_FORCE or not since we only care about
 * the least most byte.
 *
 * @TODO: 
 *
 * @param   output_file               name of file to write
 * @param   file_buffer               file buffer to write
 * @return  bool
*/

bool write_file(std::string output_file, std::vector<file_buffer_type> file_buffer)
{
    // Open the file in binary mode
    std::ofstream outfile(output_file, std::ios::binary);

    // Check if the file opened successfully
    if (!outfile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    }

    // Write the data to the file
    outfile.write(reinterpret_cast<char*>(file_buffer.data()), file_buffer.size());

    // Close the file
    outfile.close();

    return true;
}

/*
 * This function is primarily for debugging so we can track progress through out rotations.
 * I've set the 'matrix3d' as a reference so we don't copy a huge buffer.
 *
 * @TODO:
 *
 * @param   remark                  something to write before printing matrix
 * @param   matrix3d                std::vector to represent as 3d matrix and print
 * @return  void
*/
void print_matrix(std::string remark, std::vector<file_buffer_type>& matrix3d)
{
    // Reinterpret the array with different indices
    file_buffer_type(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<file_buffer_type(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(matrix3d.data());

    std::cout << remark << std::endl;
    for (uint8_t i = 0; i < RUBIX_SIDE_SIZE; i++)
    {
        for (uint8_t j = 0; j < RUBIX_SIDE_SIZE; j++)
        {
            for (uint8_t k = 0; k < RUBIX_SIDE_SIZE; k++)
                std::cout << std::hex << std::setw(8) << std::setfill('0') 
#if BRUTE_FORCE
                << int((*p)[i][j][k]) << " ";
#else
                << file_buffer_type((*p)[i][j][k]) << " ";
#endif
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
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

#ifndef DEBUG
    if (get_key(command_line_options["key_file"], key) == false)
    {
        std::cerr << "Error with key." << std::endl;
        exit(-1);
    }
#else
    key.push_back(1);
    key.push_back(2);
    key.push_back(3);
    key.push_back(0);
    key.resize(1000, 0);
#endif

    std::vector<file_buffer_type> file_buffer = { 0 };
    if (read_file(command_line_options["encrypt_file"], file_buffer) == false)
    {
        std::cerr << "Error with input file." << std::endl;
        exit(-1);
    }

#ifndef DEBUG
    for (int i = 0; i < file_buffer.size(); i++)
        file_buffer[i] ^= key[i % MAX_KEY_SIZE];
#endif

    //Build Huffman tree and generate codes
    //Node* root = buildHuffmanTree(byte_array);
    //std::map<unsigned char, std::string> codes;
    //generateCodes(root, "", codes);

    //// Print Huffman Codes
    //std::cout << "Huffman Codes:\n";
    //for (auto pair : codes) {
    //    std::cout << pair.first << " " << pair.second << "\n";
    //}

    print_matrix("start:", file_buffer);
#if BRUTE_FORCE
    shift_x(file_buffer, key, (command_line_options["decode"] == "true"));

    //print_matrix("after x:", file_buffer);

    shift_y(file_buffer, key, command_line_options["decode"] == "true");

    //print_matrix("after y:", file_buffer);

    shift_z(file_buffer, key, command_line_options["decode"] == "true");

    print_matrix("Final:", file_buffer);
#else
    // Reinterpret the array with different indices
    file_buffer_type(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<file_buffer_type(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(file_buffer.data());

    for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                uint32_t current_x_location = (*p)[z][y][x] >> X_OFFSET;
                (*p)[z][y][x] &= X_MASK;
                current_x_location = ((current_x_location + key[y]) % RUBIX_SIDE_SIZE);
                (*p)[z][y][x] |= (current_x_location << X_OFFSET);

                uint8_t x_loc = ((x - key[y]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;

                uint32_t current_y_location = ((*p)[z][y % RUBIX_SIDE_SIZE][x_loc] & ~Y_MASK) >> Y_OFFSET;
                (*p)[z][y % RUBIX_SIDE_SIZE][x_loc] &= (Y_MASK);
                current_y_location = ((current_y_location + key[x]) % RUBIX_SIDE_SIZE);
                (*p)[z][y % RUBIX_SIDE_SIZE][x_loc] |= (current_y_location << Y_OFFSET);

                uint32_t current_z_location = ((*p)[z][y][x_loc] & ~Z_MASK) >> Z_OFFSET;
                (*p)[z][y][x_loc] &= (Z_MASK);
                current_z_location = (((current_z_location + key[x]) % RUBIX_SIDE_SIZE));
                (*p)[z][y][x_loc] |= (current_z_location << Z_OFFSET);
            }

    std::sort(file_buffer.begin(), file_buffer.end());

    print_matrix("final:", file_buffer);
#endif

    std::string output_filename = command_line_options["encrypt_file"];

    std::string::size_type dot_location = output_filename.rfind('.');
    if (dot_location == std::string::npos)
        dot_location = output_filename.length();
    else
        dot_location++;

    output_filename.replace(dot_location, FILE_EXTENSION.length(), FILE_EXTENSION.c_str());

    if (write_file(output_filename, file_buffer) == false)
    {
        std::cerr << "Error writing file: " << output_filename << std::endl;
    }

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
