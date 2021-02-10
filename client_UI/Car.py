from ctypes import *

class Car(Structure) :
    _fields_ = [
        ("parkingLotNumber", c_int),
        ("occupiedNumber", c_int),
        ("carnumber", c_char * 12),
        ("startdate", c_char * 11),
        ("starttime", c_char * 9),
    ]