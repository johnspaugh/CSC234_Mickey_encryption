#include <iostream>
#include <fstream>
#include <string>
#include <AddPieToEnd.h>;

using namespace std;

int main() {
    string password = "ttt";
    int passwordNumber = 1;
    retrievePassword( password, passwordNumber);
    // std::string filePath = "../another_folder/my_text_file.txt"; // Relative path
    // std::ifstream inputFile(filePath);

    // if (inputFile.is_open()) {
    //     std::string fileContent;
    //     std::getline(inputFile, fileContent);
    //     inputFile.close();
    //     std::cout << "Content: " << fileContent << std::endl;
    // } else {
    //     std::cerr << "Unable to open file" << std::endl;
    // }

    return 0;
}