
// #include <storage/PieNumbers.txt>;
// #include <password1>;
// #include "password2";
// #include <password3>;
// #include "include/my_header.h"; 
// #include "iostream";
#include <iostream>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

string AddPieToEnd(string password, int passwordNumber){

    string ModifiedPassword = NULL;
    string pieString = NULL;
    //below example
    // string filename = "../../input.txt";
    string filenamePassword1 = "password1.txt";
    string path = NULL;
    if(passwordNumber == 1){
        path = "/storage/password1.txt";
    }else(passwordNumber == 2){
        path = "/storage/password2";
    }else(passwordNumber == 3){
        path = "/storage/password3.txt";
    }else{
        printf("did not input the correct password path, so will exit system");
        return NULL;
    }

    ModifiedPassword = readFile(path);
    pieString = readFile("/storage/PieNumbers.txt");

    
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
