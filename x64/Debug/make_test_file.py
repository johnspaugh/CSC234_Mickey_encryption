import sys

try:
    filename = sys.argv[1]
except:
    print("filename not provided.")
    exit(1)

if len(filename) == 0:
    print("filename not provided.")
    exit(1)

#for i in range(0, 130):
#    print(bytearray([i]*i).decode('latin1'))

with open(f"{filename}", "wb") as output_file:
    for _ in range(256):
        for _ in range(256):
            for i in range(256):
                output_file.write(bytearray([i]))
