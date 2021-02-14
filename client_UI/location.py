from ctypes import *

class location(Structure):
    _fields_ = [
        ("arr", c_int * 10)
    ]