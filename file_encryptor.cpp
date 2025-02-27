// fileEncryptor.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "file_encryptor.h"
#include "huffman.h"

/*
 * This in an intermediate function to create the indices in the 'rubix' array
 * we'll shuffle around.
 * 
 * @param fileBuffer                'rubix' array to add indices to
 * 
 * @return                          void
 */
void createIndices(std::vector<uint32_t>& fileBuffer)
{
    uint32_t(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<uint32_t(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(fileBuffer.data());

    /*
    * We're marking the current location of each byte in the cube array for 'shifting'
    */
    for (uint16_t z = 0; z < RUBIX_SIDE_SIZE; z++)
    {
        for (uint16_t y = 0; y < RUBIX_SIDE_SIZE; y++)
        {
            for (uint16_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                (*p)[z][y][x] |= (x << X_OFFSET);
                (*p)[z][y][x] |= (y << Y_OFFSET);
                (*p)[z][y][x] |= (z << Z_OFFSET);
            }
        }
    }
}

/*
 * This function gets the key from the specified file passed in. We create an fstream with the
 * name of the input file; we read everything in as binary. If the file can't be opened, return
 * false. After we read in the file, we verify it's at least 64 bytes. We then resize the buffer
 * to 1000 bytes, truncating the key if it's too long. If it's too short, we pad it with 'random'
 * data from the PI constant defined in the header.
 *
 * @param   inputFile               name of the file to read in
 * @param   key                     buffer to hold the key
 * @return  boolean
*/
bool getKey(std::string inputFile, std::vector<uint8_t>& keyFileBuffer)
{
    if (readFile(inputFile, keyFileBuffer, MIN_KEY_SIZE, INT_FAST32_MAX) == false)
    {
        std::cerr << "Error with key file." << std::endl;
        return false;
    }

    uint16_t keySize = static_cast<uint16_t>(keyFileBuffer.size());

    keyFileBuffer.resize(MAX_KEY_SIZE, 0);
    if (keySize < MAX_KEY_SIZE)
    {
        uint8_t start = ((keyFileBuffer[HIGHBYTE % keySize] * 10) + keyFileBuffer[LOWBYTE] % keySize);
        std::copy(PI + start, PI + start + (MAX_KEY_SIZE - keySize), keyFileBuffer.begin() + keySize);
    }

    return true;
}

/*
 * This function parses all the command line options and stores them into a map passed in.
 * Right now, it only checks for the key flag and the file flag.
 * 
 * @param   argc                    number of command line parameters directly from main
 * @param   argv                    the command line parameters directly from main
 * @param   commandLineOptions      this is the map, passed in by reference to populate
 * @return  boolean                 if we have any issue parsing the command line options
 *                                      return false, otherwise return true;
*/
bool parseOptions(int argc, char** argv, std::map<std::string, std::string>& commandLineOptions)
{
    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "-k") == 0)
        {
            if (i + 1 >= argc)
            {
                return false;
            }
            else
            {
                commandLineOptions["keyFile"] = argv[i + 1];
            }
        }
        else if (std::strcmp(argv[i], "-f") == 0)
        {
            if (i + 1 >= argc)
            {
                return false;
            }
            else
            {
                commandLineOptions["encryptFile"] = argv[i + 1];
            }
        }
        else if (std::strcmp(argv[i], "-v") == 0)
            commandLineOptions["verbose"] = "true";
        else if (std::strcmp(argv[i], "decode") == 0)
            commandLineOptions["direction"] = "decode";
    }

    if (commandLineOptions.find("direction") == commandLineOptions.end())
        commandLineOptions["direction"] = "encode";

    if (commandLineOptions.find("verbose") == commandLineOptions.end())
        commandLineOptions["verbose"] = "false";

    return true;
}

#ifdef DEBUG
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
void printMatrix(std::string remark, std::vector<FILE_BUFFER_TYPE>& matrix3d)
{
    // Reinterpret the array with different indices
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(matrix3d.data());

    std::cout << remark << std::endl;
    for (uint16_t i = 0; i < RUBIX_SIDE_SIZE; i++)
    {
        for (uint16_t j = 0; j < RUBIX_SIDE_SIZE; j++)
        {
            for (uint16_t k = 0; k < RUBIX_SIDE_SIZE; k++)
                std::cout << std::hex << std::setw(8) << std::setfill('0')
#if BRUTE_FORCE
                << int((*p)[i][j][k]) << " ";
#else
                << FILE_BUFFER_TYPE((*p)[i][j][k]) << " ";
#endif
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}
#endif      //DEBUG

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
 * @param   inputFile              name of the file to read in
 * @param   inputFileBuffer        buffer to store the file.
 * @param   maxSize                limit on file size - we can't encrypt anything larger
 *                                      than 12MB, we keep a 16MB limit on decrypting
 *                                      just in case
 * @return  boolean                if we have any issue reading the file return false, 
 *                                      otherwise return true;
*/
bool readFile(std::string inputFile, std::vector<uint8_t>& inputFileBuffer, uint32_t minSize, uint32_t maxSize)
{
    std::ifstream input(inputFile, std::ifstream::ate | std::ios::binary);

    if (!input.is_open())
    {
        std::cerr << "Can't find input file: " << inputFile << std::endl;
        return false;
    }

    std::ifstream::pos_type fileSize = input.tellg();
    if (fileSize < minSize)
    {
        std::cerr << "File too small." << std::endl;
        input.close();
        return false;
    }

    if (fileSize > maxSize)
    {
        std::cerr << "File too big." << std::endl;
        input.close();
        return false;
    }

    input.seekg(0);

    inputFileBuffer.resize(fileSize);
    input.read(reinterpret_cast<char*>(inputFileBuffer.data()), static_cast<int>(fileSize));

    input.close();

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
void shiftX(std::vector<FILE_BUFFER_TYPE>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    uint8_t keyIndex = 0;
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
void shiftY(std::vector<FILE_BUFFER_TYPE>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
    {
        uint8_t keyIndex = 0;
        for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        {
            std::vector<FILE_BUFFER_TYPE> temp(RUBIX_SIDE_SIZE);
            for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
                temp[y] = (*p)[x][y][z];

            if (decode)
                std::rotate(temp.begin(), temp.begin() + key[keyIndex], temp.end());
            else
                std::rotate(temp.rbegin(), temp.rbegin() + key[keyIndex], temp.rend());

            for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
                (*p)[x][y][z] = temp[y];
            keyIndex = (keyIndex + 1) % RUBIX_SIDE_SIZE;
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
void shiftZ(std::vector<FILE_BUFFER_TYPE>& input, std::vector<uint8_t>& key, bool decode)
{
    // Reinterpret the array with different indices
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] = reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(input.data());

    for (uint8_t z = 0; z < RUBIX_SIDE_SIZE; z++)
    {
        uint8_t keyIndex = 0;
        for (uint8_t y = 0; y < RUBIX_SIDE_SIZE; y++)
        {
            std::vector<FILE_BUFFER_TYPE> temp(RUBIX_SIDE_SIZE);
            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
                temp[x] = (*p)[x][y][z];

            if (decode)
                std::rotate(temp.begin(), temp.begin() + key[keyIndex], temp.end());
            else
                std::rotate(temp.rbegin(), temp.rbegin() + key[z], temp.rend());

            for (uint8_t x = 0; x < RUBIX_SIDE_SIZE; x++)
                (*p)[x][y][z] = temp[x];
            keyIndex = (keyIndex + 1) % RUBIX_SIDE_SIZE;
        }
    }
}
#else
/*
 * This is the main driver for encoding the file. We parse out the filename before
 * we do anything so we know what to call it later when we write the output file.
 * Per instructions, XOR the buffer against the key in 1000 chunks, followed by
 * Huffman encoding to break the byte boundry (defined in huffman.cpp).  Since we
 * have 4MB buffer to play with, we'll keep the length of the huffman encoded string
 * as well as the frequency map, which we'll need to decode this stuff. Then we do
 * our Rubix shuffle before writing the output file.
 * 
 * @param fileBuffer                std::vector buffer to encode
 * @param key                       key we'll use to shuffle the rubix array around
 * @param verbose                   boolean to track whether we want output messages
 * 
 * @return                          false, if for some reason we have an issue
 *                                  true otherwise
 */
bool encode(std::vector<uint8_t>& fileBuffer, std::vector<uint8_t>& key, bool verbose)
{
    uint8_t fileNameLength = fileBuffer[3];

    // read the file name from the buffer
    std::string outputFilename(fileBuffer.begin() + 4, fileBuffer.begin() + 4 + fileNameLength);

    // Remove the extension from the filename
    std::string::size_type dotLocation = outputFilename.rfind('.');
    if (dotLocation == std::string::npos)
    {
        outputFilename += '.';
        dotLocation = outputFilename.size();
    }
    else
        outputFilename.resize(dotLocation + 1);

    outputFilename += FILE_EXTENSION;

    if (verbose)    std::cout << "XOR file and key." << std::endl;

    /*
    * XOR the key against the array in 1K chunks (run down the full array)
    */
    XORFileAndKey(fileBuffer, key);

    if (verbose)    std::cout << "Huffman Encoding." << std::endl;

    /*
    * Perform Huffman encoding of resulting array, creating array�
    */
    std::array<uint32_t, 256> freq = { 0 };
    std::vector<uint8_t> encodedBytes;
    uint32_t stringLength = 0;
    if (huffmanEncode(fileBuffer, freq, encodedBytes, stringLength) == false)
    {
        std::cerr << "Error with huffman encoding" << std::endl;
        exit(1);
    }

    if (verbose)    std::cout << "Rubix shuffle." << std::endl;

    /*
    * Load array' into the Rubix array
    */
    std::vector<uint32_t> rubix = { 0 };
    std::copy(encodedBytes.begin(), encodedBytes.end(), std::back_inserter(rubix));
    rubix.erase(rubix.begin());

    /*
    * Let's clear these vectors since we don't need them anymore.
    * They'll probably be cleaned up with the garbage collection, but we're
    * now using 64 MB of memory and these are unnecessary.
    */
    encodedBytes.clear();
    fileBuffer.clear();

    rubix.resize(SIXTEEN_MEGABYTES);

    /*
    * we need to keep the frequency map to huffman decode, since we're not using the last
    * 4 megabytes, we'll just stick it there
    */
    for (uint16_t i = 0; i < freq.size(); i++)
        for (uint8_t j = 0; j < sizeof(uint32_t); j++)
            rubix[rubix.size() - 1024 + (i * 4) + j] = uint32_t(freq[i] >> (j * 8)) & 0xff;


    /*
    * we also need to keep the length of the huffman encoded string to pass back to the decoder
    * we'll just stick it right before the frequency map
    */
    for (uint8_t j = 0; j < sizeof(uint32_t); j++)
        rubix[rubix.size() - 1028 + j] = uint32_t(stringLength >> (j*8)) & 0xff;

    createIndices(rubix);

    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(rubix.data());

    /*
    * 'Rubix' shift array
    */
    for (uint16_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        for (uint16_t y = 0; y < RUBIX_SIDE_SIZE; y++)
            for (uint16_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                uint32_t currentXLocation = (*p)[z][y][x] >> X_OFFSET;
                (*p)[z][y][x] &= X_MASK;
                currentXLocation = ((currentXLocation + key[y]) % RUBIX_SIDE_SIZE);
                (*p)[z][y][x] |= (currentXLocation << X_OFFSET);

                int16_t xLoc = ((x - key[y]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;

                uint32_t currentYLocation = ((*p)[z][y % RUBIX_SIDE_SIZE][xLoc] & ~Y_MASK) >> Y_OFFSET;
                (*p)[z][y % RUBIX_SIDE_SIZE][xLoc] &= (Y_MASK);
                currentYLocation = ((currentYLocation + key[x]) % RUBIX_SIDE_SIZE);
                (*p)[z][y % RUBIX_SIDE_SIZE][xLoc] |= (currentYLocation << Y_OFFSET);

                uint32_t currentZLocation = ((*p)[z][y][xLoc] & ~Z_MASK) >> Z_OFFSET;
                (*p)[z][y][xLoc] &= (Z_MASK);
                currentZLocation = (((currentZLocation + key[x]) % RUBIX_SIDE_SIZE));
                (*p)[z][y][xLoc] |= (currentZLocation << Z_OFFSET);
            }

    std::sort(rubix.begin(), rubix.end());

    if (verbose)    std::cout << "Write encrypted file." << std::endl;

    /*
     * write output file
     */
    if (writeFile<uint32_t>(outputFilename, rubix) == false)
    {
        std::cerr << "Error writing file." << std::endl;
        return false;
    }

    return true;

}

/*
 * This is the main driver to decode the file. We should be doing the reverse order of
 * the encode function. Start with the Rubix shift. For the huffman decoding, we need 
 * to extract the frequecy map and length of the encoded string. Then, build the encoded
 * string back from the least significant bytes of the Rubix array, then XOR the decoded
 * bytes against the key, followed by writing the output file.
 * 
 * @param fileBuffer                std::vector containing input file to decode
 * @param key                       key we'll use to shuffle the rubix array around
 * @param verbose                   boolean to track whether we want output messages
 *
 * @return                          false, if for some reason we have an issue
 *                                  true otherwise
 */
bool decode(std::vector<uint8_t>& fileBuffer, std::vector<uint8_t>& key, bool verbose)
{
    /*
     * 3. Perform steps 9 & 10 to build the Shuffle map
     * 4. Reverse step 11 � move elements from the input array into the Rubix array
     */ 

    if (verbose)    std::cout << "Rubix shuffle." << std::endl;


     /*
      * Load array' into the Rubix array
      */
    std::vector<uint32_t> rubix;
    std::copy(fileBuffer.begin(), fileBuffer.end(), std::back_inserter(rubix));

    createIndices(rubix);

    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(rubix.data());

    uint32_t debug = 0;
    for (int16_t z = 0; z < RUBIX_SIDE_SIZE; z++)
    {
        for (int16_t y = 0; y < RUBIX_SIDE_SIZE; y++)
        {
            for (int16_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                int32_t currentZLocation = ((*p)[z][y][x] & ~Z_MASK) >> Z_OFFSET;
                (*p)[z][y][x] &= (Z_MASK);
                currentZLocation = ((currentZLocation - key[x]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[z][y][x] |= (currentZLocation << Z_OFFSET);

                int32_t currentYLocation = ((*p)[z][y][x] & ~Y_MASK) >> Y_OFFSET;
                (*p)[z][y][x] &= (Y_MASK);
                currentYLocation = ((currentYLocation - key[x]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[z][y][x] |= (currentYLocation << Y_OFFSET);

                int16_t yLoc = (y + key[x]) % RUBIX_SIDE_SIZE
                    , zLoc = (z + key[x]) % RUBIX_SIDE_SIZE;

                int32_t currentXLocation = ((*p)[zLoc][yLoc][x] & ~X_MASK) >> X_OFFSET;
                (*p)[zLoc][yLoc][x] &= X_MASK;
                currentXLocation = ((currentXLocation - key[y]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[zLoc][yLoc][x] |= (currentXLocation << X_OFFSET);
            }
        }
    }

    std::sort(rubix.begin(), rubix.end());

    /*
     * 9. Perform Huffman decoding to create array from array (implement last)
     */
    std::array<uint32_t, 256> freq = { 0 };
    for (int i = 0; i < freq.size(); i++)
    {
        size_t start = rubix.size() - 1024 + (i * 4);
        for (size_t j = start; j < start + 4; j++)
            freq[i] |= (rubix[j]&0xff) << ((j % 4) * 8);
    }

    uint32_t stringLength = 0;
    for (size_t j = rubix.size()-1028; j < rubix.size()-1024; j++)
        stringLength |= (rubix[j] & 0xff) << ((j % 4) * 8);

    std::vector<uint8_t> decodedBytes;
    std::string input;
    
    for (uint32_t i : rubix)
    {
        input += std::bitset<8>(i&0xff).to_string();
        if (input.length() >= stringLength)
            break;
    }

    input.resize(stringLength);

    if (verbose)    std::cout << "Huffman decoding." << std::endl;

    if ( huffmanDecode(input, freq, decodedBytes) ==false)
    {
        std::cerr << "Error with huffman encoding" << std::endl;
        exit(1);
    }

    if (verbose)    std::cout << "XOR file and key." << std::endl;

    /*
     * 10. Perform encrypt step 3 (XOR)
     * XOR the key against the array in 1K chunks (run down the full array)
     */
    XORFileAndKey(decodedBytes, key);

    /* 
     * 11. Extract string length and file suffix
     *      3 bytes for file size
     *      1 byte for file name length
     *      file name
     */

    // read 3 bytes for the size of the file.
    uint32_t fileSize = 0;
    for (int i = 2; i >= 0; i--)
        fileSize |= (decodedBytes[i] << ((2-i)*8));

    uint8_t fileNameLength = decodedBytes[3];

    // read the file name from the buffer
    std::string outputFilename(decodedBytes.begin() + 4, decodedBytes.begin() + 4 + fileNameLength);

    /*
    * Erase the 3 byte header and file name from the buffer
    */
    decodedBytes.erase(decodedBytes.begin(), decodedBytes.begin() + 4 + fileNameLength);
    decodedBytes.erase(decodedBytes.begin() + fileSize, decodedBytes.end());

    if (verbose)    std::cout << "Write decrypted file." << std::endl;

    /*
     * 12. Create output file with correct suffix using string length
     */
    outputFilename = "1" + outputFilename;
    if (writeFile<uint8_t>(outputFilename, decodedBytes) == false)
    {
        std::cerr << "Error writing file." << std::endl;
        return false;
    }

    return true;
}
#endif      //BRUTE_FORCE

/*
 * This function writes the encripted file to disk. I've templated it 
 * to eliminate duplicate work. The encode function calls this with 
 * uint32_t data type, decode with uint8_t data type. In either case,
 * we only care about the least significant byte.
 *
 * @param   outputFile               name of file to write
 * @param   fileBuffer               file buffer to write
 * @return  bool
*/
template <typename T>
bool writeFile(std::string outputFile, std::vector < T > & fileBuffer)
{
    // Open the file in binary mode
    std::ofstream outfile(outputFile, std::ios::binary);

    // Check if the file opened successfully
    if (!outfile.is_open()) 
    {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    }

    // Write the data to the file
    // We're only interested in the least significant byte to put in to the file
    for (T byte : fileBuffer)
        outfile.put(byte & 0xff);

    // Close the file
    outfile.close();

    return true;
}

/*
 * This function writes the XORs the file to encrypt with the key in 1000 byte chunks using
 * MAX_KEY_SIZE defined in the header.
 *
 * @TODO:
 *
 * @param   fileBuffer               file buffer to write
 * @param   key                       prepared key
 * @return  bool
*/
void XORFileAndKey(std::vector<uint8_t>& fileBuffer, std::vector<uint8_t>& key)
{
    int i = 0;
    for (auto b : fileBuffer)
    {
        b ^= key[i];
        i = (i + 1) % MAX_KEY_SIZE;
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
    std::map<std::string, std::string> commandLineOptions;

    if (parseOptions(argc, argv, commandLineOptions) == false)
    {
        std::cout << "Invalid options" << std::endl;
        exit(-1);
    }

    /*
     * Take key and truncate or fill to make it a full 1K
     */
    std::vector<uint8_t> key;
    if (getKey(commandLineOptions["keyFile"], key) == false)
    {
        std::cerr << "Error with key." << std::endl;
        exit(-1);
    }

    /* Take input file and load into a linear array.  At the start of the array include 
     * information about the length of the string extracted from the file (4 bytes) and the 
     * file suffix.  Pad out information beyond the end of the string with random bytes from 
     * the original string to fill out the array to 16Mb.  Avoids strong pattern marking 
     * end of cleartext 
     */
    std::vector<uint8_t> fileBuffer = { 0 };
    int maxSize = (commandLineOptions["direction"] == "encode") ? TWELVE_MEGABYTES : SIXTEEN_MEGABYTES;

    if (readFile(commandLineOptions["encryptFile"], fileBuffer, 0, maxSize) == false)
    {
        std::cerr << "Error with input file." << std::endl;
        exit(-1);
    }


    if (commandLineOptions["direction"] == "encode")
    {
        /*
         * write 3 bytes for the size of the file. I forgot my reasoning on why the byte
         * order is like this, we only 3 bytes be cause 12MB < 2^32
         */
        uint32_t fileSize = static_cast<uint32_t>(fileBuffer.size());
        for (int i = 2; i >= 0; i--)
            fileBuffer.insert(fileBuffer.begin() + (2 - i), (fileSize >> (i * 8)) & 0xff);

        fileBuffer.insert(fileBuffer.begin() + 3, (uint8_t)commandLineOptions["encryptFile"].size());

        /*
         * write the file name to the beginning of the buffer so we can extract it later
         */
        fileBuffer.insert(fileBuffer.begin() + 4, commandLineOptions["encryptFile"].begin(), commandLineOptions["encryptFile"].end());

        //TODO: pad input file with 'random noise'

        if (encode(fileBuffer, key, commandLineOptions["verbose"] == "true") == false)
        {
            std::cerr << "Error encoding file." << std::endl;
            exit(1);
        }
    }
    else
    {
        if (decode(fileBuffer, key, commandLineOptions["verbose"] == "true") == false)
        {
            std::cerr << "Error encoding file." << std::endl;
            exit(1);
        }
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
