#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

void encrypt_file(const std::string& file_path, const std::string& key) {
    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open file for reading.\n";
        return;
    }

    std::string output_path = file_path + ".enc";
    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        std::cerr << "Error: Could not open file for writing.\n";
        return;
    }

    char buffer;
    size_t key_len = key.length();
    size_t i = 0;

    while (input.get(buffer)) {
        buffer ^= key[i % key_len];  // XOR with key
        output.put(buffer);
        i++;
    }

    input.close();
    output.close();

    std::cout << "File encrypted and saved as " << output_path << "\n";
}

void decrypt_file(const std::string& file_path, const std::string& key) {
    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open file for reading.\n";
        return;
    }

    std::string output_path = file_path;
    if (output_path.ends_with(".enc")) {
        output_path.replace(output_path.length() - 4, 4, ".dec");
    } else {
        output_path += ".dec";
    }

    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        std::cerr << "Error: Could not open file for writing.\n";
        return;
    }

    char buffer;
    size_t key_len = key.length();
    size_t i = 0;

    while (input.get(buffer)) {
        buffer ^= key[i % key_len];  // XOR with key again to decrypt
        output.put(buffer);
        i++;
    }

    input.close();
    output.close();

    std::cout << "File decrypted and saved as " << output_path << "\n";
}

int main() {
    while (true) {
        std::cout << "\nWelcome to the Mickey Encryption Tool!\n";
        std::cout << "1. Encrypt a file\n";
        std::cout << "2. Decrypt a file\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") {
            std::string file_path, key;
            std::cout << "Enter the path of the file to encrypt: ";
            std::getline(std::cin, file_path);

            if (!std::filesystem::exists(file_path)) {
                std::cerr << "Error: File not found.\n";
                continue;
            }

            std::cout << "Enter the encryption key: ";
            std::getline(std::cin, key);

            encrypt_file(file_path, key);
        }
        else if (choice == "2") {
            std::string file_path, key;
            std::cout << "Enter the path of the file to decrypt: ";
            std::getline(std::cin, file_path);

            if (!std::filesystem::exists(file_path)) {
                std::cerr << "Error: File not found.\n";
                continue;
            }

            std::cout << "Enter the decryption key: ";
            std::getline(std::cin, key);

            decrypt_file(file_path, key);
        }
        else if (choice == "3") {
            std::cout << "Exiting... Goodbye!\n";
            break;
        }
        else {
            std::cerr << "Invalid choice. Please enter 1, 2, or 3.\n";
        }
    }

    return 0;
}
