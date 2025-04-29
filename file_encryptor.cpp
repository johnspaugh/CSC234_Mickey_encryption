// fileEncryptor.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "file_encryptor.h"
#include "huffman.h"



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

    int keySize = static_cast<int>(keyFileBuffer.size());
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
                    << FILE_BUFFER_TYPE((*p)[i][j][k]) << " ";
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

/*
 * This function finds a prime number for the final shuffle. In the event the prime
 * number is a factor of the maximum file size, we'll keep increasing until we find a 
 * suitable value.
 * 
 *
 * @param index                     start at the specified value from the key
 *
 * @return                          prime number not a factor of maximum file size
 */
uint32_t getPrime(uint8_t index)
{
    while (SIXTEEN_MEGABYTES % primes[index] == 0)
        index++;

    return primes[index];
}

/*
 * This function adds random values to the buffer. Random values adds another layer
 * of security as it hides the length of encoded bytes.
 * 
 * @param vec                       std::vector buffer to pad
 * @param pos                       start postion for adding random values
 *
 * @return                          void
*/
void addPadding(std::vector<uint32_t>& vec, uint32_t pos)
{
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, 0xff);

    for (uint32_t i = pos; i < SIXTEEN_MEGABYTES - META_DATA_SIZE; i++)
        vec[i] = distrib(gen);
}

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

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    /*
     * XOR the key against the array in 1K chunks (run down the full array)
     */
    XORFileAndKey(fileBuffer, key);

    if (verbose)    std::cout << "Huffman Encoding." << std::endl;

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    /*
     * Perform Huffman encoding of resulting array, creating array
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

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

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
            rubix[SIXTEEN_MEGABYTES - 1024 + (i * 4) + j] = uint32_t(freq[i] >> (j * 8)) & 0xff;

    addPadding(rubix, stringLength);

    /*
     * we also need to keep the length of the huffman encoded string to pass back to the decoder
     * we'll just stick it right before the frequency map
     */
    for (uint8_t j = 0; j < sizeof(uint32_t); j++)
        rubix[SIXTEEN_MEGABYTES - META_DATA_SIZE + j] = uint32_t(stringLength >> (j*8)) & 0xff;

    // Cast the buffer to a pointer instead of loading to a separate array
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(rubix.data());

    /*
     * 'Rubix' shift array
     */
    for (uint16_t z = 0; z < RUBIX_SIDE_SIZE; z++)
        for (uint16_t y = 0; y < RUBIX_SIDE_SIZE; y++)
            for (uint16_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                uint32_t currentXLocation =((x + key[y]) % RUBIX_SIDE_SIZE);
                (*p)[z][y][x] |= (currentXLocation << X_OFFSET);

                int16_t xLoc = ((x - key[y]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;

                uint32_t currentYLocation = ((y + key[x]) % RUBIX_SIDE_SIZE);
                (*p)[z][y % RUBIX_SIDE_SIZE][xLoc] |= (currentYLocation << Y_OFFSET);

                uint32_t currentZLocation = (((z + key[x]) % RUBIX_SIDE_SIZE));
                (*p)[z][y][xLoc] |= (currentZLocation << Z_OFFSET);
            }

    std::sort(rubix.begin(), rubix.end());

    if (verbose)    std::cout << "Shuffle array." << std::endl;

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    /*
     * This is the final shuffle in the encryption. We put a byte in to an empty slot based on a prime
     * number selected from the primes array and the 59th byte from the key.
     */
    uint32_t prime = getPrime(key[59]);
    uint32_t counter = 1;
    for (uint32_t i = 1; i < rubix.size(); i++)
    {
        rubix[rubix.size() - (i * prime) % rubix.size()] &= 0xff;
        rubix[rubix.size() - (i * prime) % rubix.size()] |= counter++ << 8;
    }
    std::sort(rubix.begin(), rubix.end());

    if (verbose)    std::cout << "Write encrypted file." << std::endl;

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    /*
     * write output file
     */
    if (writeFile<uint32_t>(outputFilename, rubix) == false)
    {
        std::cerr << "Error writing file." << std::endl;
        return false;
    }

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif
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
     * Load array' into the Rubix array
     */
    std::vector<uint32_t> rubix;
    std::copy(fileBuffer.begin(), fileBuffer.end(), std::back_inserter(rubix));

    /*
     * 3. Perform steps 9 & 10 to build the Shuffle map
     * 4. Reverse step 11 - move elements from the input array into the Rubix array
     */ 
    if (verbose)    std::cout << "Unshuffle array." << std::endl;

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    uint32_t prime = getPrime(key[59]);
    uint32_t counter = 1;
    for (uint32_t i = 1; i < rubix.size(); i++)
        rubix[counter++] |= (rubix.size() - ((i * prime) % rubix.size())) << 8;

    std::sort(rubix.begin(), rubix.end());

    if (verbose)    std::cout << "Rubix shuffle." << std::endl;
#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    /*
     * 'Rubix' unshuffling, in place
     */

    // We, again, cast the buffer to a pointer instead of loading to a separate array
    FILE_BUFFER_TYPE(*p)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE] =
        reinterpret_cast<FILE_BUFFER_TYPE(*)[RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE][RUBIX_SIDE_SIZE]>(rubix.data());

    for (int16_t z = 0; z < RUBIX_SIDE_SIZE; z++)
    {
        for (int16_t y = 0; y < RUBIX_SIDE_SIZE; y++)
        {
            for (int16_t x = 0; x < RUBIX_SIDE_SIZE; x++)
            {
                (*p)[z][y][x] &= (Z_MASK);
                int32_t currentZLocation = ((z - key[x]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[z][y][x] |= (currentZLocation << Z_OFFSET);

                (*p)[z][y][x] &= (Y_MASK);
                int32_t currentYLocation = ((y - key[x]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[z][y][x] |= (currentYLocation << Y_OFFSET);

                int16_t yLoc = (y + key[x]) % RUBIX_SIDE_SIZE
                    , zLoc = (z + key[x]) % RUBIX_SIDE_SIZE;

                (*p)[zLoc][yLoc][x] &= X_MASK;
                int32_t currentXLocation = ((x - key[y]) + RUBIX_SIDE_SIZE) % RUBIX_SIDE_SIZE;
                (*p)[zLoc][yLoc][x] |= (currentXLocation << X_OFFSET);
            }
        }
    }

    std::sort(rubix.begin(), rubix.end());

    /*
     * 9. Perform Huffman decoding to create array from array (implement last)
     */
    if (verbose)    std::cout << "Huffman decoding." << std::endl;
#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    // we need to get the frequency map from the end of the buffer as Huffman can't
    // recreate it
    std::array<uint32_t, 256> freq = { 0 };
    for (int i = 0; i < freq.size(); i++)
    {
        size_t start = rubix.size() - 1024 + (i * 4);
        for (size_t j = start; j < start + 4; j++)
            freq[i] |= (rubix[j]&0xff) << ((j % 4) * 8);
    }

    // We need to get the length of the huffman encoded string so we know how much
    // to truncate before passing it to the decode function
    uint32_t stringLength = 0;
    for (size_t j = rubix.size()-1028; j < rubix.size()-1024; j++)
        stringLength |= (rubix[j] & 0xff) << ((j % 4) * 8);

    // create the Huffman string to decode
    std::vector<uint8_t> decodedBytes;
    std::string input;
    
    for (uint32_t i : rubix)
    {
        input += std::bitset<8>(i&0xff).to_string();
        if (input.length() >= stringLength)
            break;
    }

    input.resize(stringLength);


    if ( huffmanDecode(input, freq, decodedBytes) ==false)
    {
        std::cerr << "Error with huffman encoding" << std::endl;
        exit(1);
    }

    if (verbose)    std::cout << "XOR file and key." << std::endl;

#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

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
    if (verbose)    std::cout << "Write decrypted file." << std::endl;
#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

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


    /*
     * 12. Create output file with correct suffix using string length
     */
    if (writeFile<uint8_t>(outputFilename, decodedBytes) == false)
    {
        std::cerr << "Error writing file." << std::endl;
        return false;
    }
#if TIMER
    times.push_back(std::chrono::steady_clock::now());
#endif

    return true;
}

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
    /*
     * Check if output file already exists. If it does, do we want to 
     * overwrite it or rename it?
     * 
     * C++ doesn't have a portable way to only get the character without
     * user pressing ENTER, so we have to make sure any extra characters
     * don't interfere.
     */
    while (std::filesystem::exists(outputFile))
    {
        std::cout << outputFile << " already exists.";
        int ch;
        do
        {
            std::cout << "\n[O]verwrite or [R]ename ? ";
            ch = toupper(std::getchar());

            // Ignore to the end of line
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // reset the stream state.
            std::cin.clear();
        } while ((ch != 'O') && (ch != 'R'));

        if (ch == 'O')
            break;

        std::cout << "Enter new Filename: ";
        std::getline(std::cin, outputFile);

        // reset the stream state.
        std::cin.clear();
    }

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

#if TIMER
/*
 * This function prints the time for each stage.
 * 
 * @param   none
 * @return  none
 */
void writeTimeStats()
{
    std::array<std::string, 6> labels = { {"XOR = ", "HUFFMAN = ","RUBIX = ","SHUFFLE = ","WRITE = "} };
    std::cout << std::fixed << std::setprecision(9) << std::left;

    for (uint8_t i = 1; i < times.size(); i++)
    {
        std::chrono::duration<double> diff = times[i] - times[i-1];
        std::cout << diff << '\t';
    }

    std::chrono::duration<double> diff = times[times.size()-1] - times[0];
    std::cout << diff << '\n';
}
#endif
/*
 * This function writes the XORs the file to encrypt with the key in 1000 byte chunks using
 * MAX_KEY_SIZE defined in the header.
 *
 * @param   fileBuffer               file buffer to write
 * @param   key                       prepared key
 * @return  bool
*/
void XORFileAndKey(std::vector<uint8_t>& fileBuffer, std::vector<uint8_t>& key)
{
    int i = 0;
    for (uint8_t &b : fileBuffer)
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
    int maxSize = (commandLineOptions["direction"] == "encode") ? TWELVE_MEGABYTES : SIXTEEN_MEGABYTES
      , minSize = (commandLineOptions["direction"] == "encode") ? 0 : SIXTEEN_MEGABYTES;

    if (readFile(commandLineOptions["encryptFile"], fileBuffer, minSize, maxSize) == false)
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

        if (encode(fileBuffer, key, commandLineOptions["verbose"] == "true") == false)
        {
            std::cerr << "Error encoding file." << std::endl;
            exit(1);
        }
#if TIMER
        writeTimeStats();
#endif
    }
    else
    {
        if (decode(fileBuffer, key, commandLineOptions["verbose"] == "true") == false)
        {
            std::cerr << "Error encoding file." << std::endl;
            exit(1);
        }
#if TIMER
        writeTimeStats();
#endif
    }

    return 0;
}
