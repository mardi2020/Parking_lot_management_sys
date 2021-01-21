from socket import *
from ctypes import *

TOTAL = 10
PORT = 12345

class ParkingLot(Structure) :
    _fields_ = [
        ("parkingLotNumber", c_int),
        ("occupiedNumber", c_int),
        ("totalNumber", c_int),
        ("occupiedarea", c_int * TOTAL)
            ]

socket_client = socket(AF_INET, SOCK_STREAM)
socket_client.connect(('127.0.0.1', PORT))

while True :
    buff = socket_client.recv(sizeof(ParkingLot))
    print(buff)
    pl = ParkingLot.from_buffer_copy(buff)
    print(pl.parkingLotNumber)
    print(pl.occupiedNumber)
    print(pl.totalNumber)
    print(pl.occupiedarea)

print("Closing socket")
socket_client.close()
    
