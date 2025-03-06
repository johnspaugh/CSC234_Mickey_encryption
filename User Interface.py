import os

def encrypt_file(file_path, key):
    # Placeholder for encryption logic
    print(f"Encrypting {file_path} with key {key}...")
    output_path = file_path + ".enc"
    print(f"File encrypted and saved as {output_path}\n")

def decrypt_file(file_path, key):
    # Placeholder for decryption logic
    print(f"Decrypting {file_path} with key {key}...")
    output_path = file_path.replace(".enc", ".dec")
    print(f"File decrypted and saved as {output_path}\n")

def main():
    while True:
        print("\nWelcome to the Mickey Encryption Tool!")
        print("1. Encrypt a file")
        print("2. Decrypt a file")
        print("3. Exit")
        
        choice = input("Enter your choice: ")
        
        if choice == "1":
            file_path = input("Enter the path of the file to encrypt: ")
            if not os.path.exists(file_path):
                print("Error: File not found.")
                continue
            key = input("Enter the encryption key: ")
            encrypt_file(file_path, key)
        
        elif choice == "2":
            file_path = input("Enter the path of the file to decrypt: ")
            if not os.path.exists(file_path):
                print("Error: File not found.")
                continue
            key = input("Enter the decryption key: ")
            decrypt_file(file_path, key)
        
        elif choice == "3":
            print("Exiting... Goodbye!")
            break
        
        else:
            print("Invalid choice. Please enter 1, 2, or 3.")

if __name__ == "__main__":
    main()
