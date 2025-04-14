# **FILE ENCRYPTOR**
This is a general purpose file encryptor to encrypt a file up to 12MB in size.

##USAGE
file_encryptor [-v decode] -k <key file name> -f <file to encrypt>
	-v						verbose output, will print which stage of encryption/decryption
	decode					decode flag, use to decrypt file
	-k <key file name>		file name of the key used for encryption/decryption must be at least 64 bytes
	-f <file to encrypt>	file name of the file to encrypt/decrypt