from ctypes import *

class array(Structure) :
    _fields_ = [
        ("arr", c_int * 10)
    ]