

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> 
#include "../include/AddPieToEnd.h" 
// #include "filename" is used for local header files, which are made by you.
// #include <filename> is used for header files Globally included in C++, System header files
// there is no syntax like <"filename">
// $ g++ "./src/main.cpp" "./src/AddPieToEnd.cpp" -o main.exe

using namespace std;
namespace fs = std::filesystem;

string AddPieToEnd(string password){

    string passwordPadding = NULL;
    string pieString = readFile("/storage/LittlePieNumbers1K.txt");

    passwordPadding = password + pieString;
    string result = truncateString(passwordPadding);

    return result;
}

// std::string truncateString(const std::string& inputString) {
string truncateString(string inputString){
   
    //1KB = (1024 bytes)
    const size_t maxSize = 1024; 
    if (inputString.length() <= maxSize) {
        //in theory should never have <= input, so message error
        // cout 
        cerr << "the input is <= 1KB, should not happen when padding with pie." << endl;
        return inputString;
    } else {
        return inputString.substr(0, maxSize);
    }
}

string retrievePassword(string password, int passwordNumber){
    
    cout << "Testing 2 " << endl;    
    string ModifiedPassword = "";
    string pieString = "";
    //below example
    // string filename = "../../input.txt";
    string filenamePassword1 = "password1.txt";
    string path = "";
    cout << "Testing 3 " << endl;
    if(passwordNumber == 1){
        // string of password
        path = "/storage/password1.txt";
    }else if(passwordNumber == 2){
        //this was picture
        path = "/storage/password2";
    }else if(passwordNumber == 3){
        // string of password
        path = "/storage/password3.txt";
    }else{
        path = "empty";
        // cout 
        cerr << "did not input the correct password path, so will exit system" << endl;
        return ""; //NULL;
    }

    ModifiedPassword = readFile(path);
    ModifiedPassword = AddPieToEnd(ModifiedPassword);
    
    return ModifiedPassword;
}

string readFile(string path){

    std::string output;
    //std::string filePath = "../another_folder/my_text_file.txt"; // Relative path
    std::string filePath = path; // "/storage/password1.txt";
    std::ifstream inputFile(filePath);

    if (inputFile.is_open()) {
        cout << "Testing 4 " << endl;
        std::string fileContent;
        std::getline(inputFile, fileContent);
        inputFile.close();
        std::cout << "Content: " << fileContent << std::endl;
        output = fileContent; 
        // std::cout << output << std::endl;
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

    //return 0;
    return output;
}
