import binascii
import serial
import sys


def main():
    with serial.Serial(sys.argv[3], 115200) as ser:
        with open(sys.argv[1], 'rb') as f:
            ser.write('loadimg\n'.encode('ascii'))
            print(ser.readline().decode(), end='')
            f.seek(0, 2)
            ser.write(f.tell().to_bytes(4, byteorder='big', signed=False))
            print(ser.readline().decode(), end='')
            ser.write(int(sys.argv[2], 16).to_bytes(4, byteorder='big', signed=False))
            print(ser.readline().decode(), end='')
            f.seek(0)
            crc = binascii.crc32(f.read())
            ser.write(crc.to_bytes(4, byteorder='big', signed=False))
            f.seek(0)
            ser.write(f.read())

        print(ser.readline().decode(), end='')
        print(ser.readline().decode(), end='')
        print(ser.readline().decode(), end='')

if __name__ == '__main__':
    main()
