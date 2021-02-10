from ctypes import *

class message_format(Structure) :
    _fields_ = [
        ("message_id", c_short),
        #("sub_id", c_short),
        ("len", c_int),
        ("data", c_char * 256)
    ]