from time import sleep
import serial
from tqdm import tqdm
from sys import argv
import os.path
# pip3 install pyserial


def readline():
    ret = b''
    t = ser.read()
    while t != b'\n':
        ret += t
        t = ser.read()
    return ret[:-1]


def recv_and_print():
    sleep(1)
    print(readline().decode())


device = '/dev/ttys005' if len(argv) == 1 else argv[1]
print("[+]", device)
baud_rate = 115200

kernel_path = 'kernel/kernel.img'
data = open('kernel/kernel.img', 'rb', buffering=0).read()
size = len(data)

'''Start sending'''
while not os.path.exists(device):
    pass
ser = serial.Serial(device, baud_rate)

# magic
print("[-]", "Sending magic")
ser.write(b'M30W')
ser.flush()
recv_and_print()

# size
print("[-]", "Sending size")
ser.write(int.to_bytes(size, 4, byteorder='little'))
ser.flush()
recv_and_print()

# data
print("[-]", "Sending data")
for el in tqdm(data):
    ser.write(int.to_bytes(el, length=1, byteorder='little'))
    ser.flush()

print("[-]", f"Finish :-)")
recv_and_print()
