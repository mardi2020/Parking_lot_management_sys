from ctypes import *

class ParkingLot(Structure) :
    _fields_ = [
        ("number", c_int),
        ("address", c_char * 256),
        ("empty", c_int), 
        ("space", c_int),
        ("occupiedarea", c_int * 10)
    ]