
#include <stdlib> //<stdlib.h> if wanted to hold in the include file.
#include <iostream>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

string AddPieToEnd(string password){

    string passwordPadding = NULL;
    string pieString = readFile("/storage/LittlePieNumbers1K.txt");

    passwordPadding = password + pieString;
    string result = truncateString(passwordPadding);

    return result;
}

std::string truncateString(const std::string& inputString) {
    //1KB = (1024 bytes)
    const size_t maxSize = 1024; 
    if (inputString.length() <= maxSize) {
        //in theory should never have <= input, so message error
        printf("the input is <= 1KB, should not happen when padding with pie.");
        return inputString;
    } else {
        return inputString.substr(0, maxSize);
    }
}

string retrievePassword(string password, int passwordNumber){

    string ModifiedPassword = NULL;
    string pieString = NULL;
    //below example
    // string filename = "../../input.txt";
    string filenamePassword1 = "password1.txt";
    string path = NULL;
    if(passwordNumber == 1){
        // string of password
        path = "/storage/password1.txt";
    }else(passwordNumber == 2){
        //this was picture
        path = "/storage/password2";
    }else(passwordNumber == 3){
        // string of password
        path = "/storage/password3.txt";
    }else{
        printf("did not input the correct password path, so will exit system");
        return NULL;
    }

    ModifiedPassword = readFile(path);
    ModifiedPassword = AddPieToEnd(ModifiedPassword);
    
    return ModifiedPassword;
}

string readFile(string path){

    string output;
    //std::string filePath = "../another_folder/my_text_file.txt"; // Relative path
    std::string filePath = path; // "/storage/password1.txt";
    std::ifstream inputFile(filePath);

    if (inputFile.is_open()) {
        std::string fileContent;
        std::getline(inputFile, fileContent);
        inputFile.close();
        std::cout << "Content: " << fileContent << std::endl;
        output = fileContent;
        printf(output);
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

    //return 0;
    return output;
}
