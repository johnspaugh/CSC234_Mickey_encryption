#include <iostream>
#include <fstream>
#include <string>
#include "../include/AddPieToEnd.h" 
// #include <AddPieToEnd.h>

using namespace std;

int main() {
    string password = "ttt";
    int passwordNumber = 1;
    string passwordretrived = "";
    cout << "Testing "<< endl;
    passwordretrived = retrievePassword( password, passwordNumber);
    
    cout << passwordretrived << endl;

    return 0;
}