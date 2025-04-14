# **FILE ENCRYPTOR**

This is a general purpose file encryptor to encrypt a file up to 12MB in size. Encryption is done by a proprietary algorithm of XOR, Huffman coding, 'Rubix' shifting, and final shuffle. 

User keys must be at least 64 bytes and no more than 1000. Keys longer than 1000 bytes are truncated.

Input files can be of any size up to 12MB. Filenames including spaces must be in quotes.

The input file is XOR'd with the key, encoded using the Huffman algorithm to break byte boundary, then loaded into a 3D cube. The bytes in the cube are shifted along each of the axes according to the input key. The final shuffle is based on a predefined prime number.

  

## USAGE
> file_encryptor [-v decode] -k <key_file_name> -f <file_to_encrypt>

- 	-v 		verbose output, will print which stage of encryption/decryption, optional
-	decode 		decode flag, use to decrypt file
- -k <key file name>		file name of the key used for encryption/decryption must be at least 64 bytes
-	-f <file to encrypt>	file name of the file to encrypt/decrypt
=======
