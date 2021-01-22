from socket import *
from ctypes import *

TOTAL = 10
PORT = 12345 

class Car(Structure) :
    _fields_ = [
        ("parkingLotNumber", c_int),
        ("occupiedNumber", c_int),
        ("carnumber", c_char * 100)
            ]

class message_format(Structure) :
    _fields_ = [
        ("message_id", c_short),
        ("len", c_int),
        ("data", c_char * 256)
    ]

class ParkingLot(Structure) :
    _fields_ = [
        ("number", c_int),
        ("address", c_char * 256),
        ("empty", c_int), 
        ("space", c_int),
        ("occupiedarea", c_int * TOTAL)
    ]


        

socket_client = socket(AF_INET, SOCK_STREAM)
socket_client.connect(('127.0.0.1', PORT))

# 차량 정보를 얻기 위한 메시지
mess = message_format()
mess.message_id = 2 # 주차장 : 1 차량 : 2
data ="11가 1111"
mess.len = len(data)
mess.data = bytes(data+"\0",'utf-8')

socket_client.send(mess)


if mess.message_id == 1 :
    buff = socket_client.recv(sizeof(ParkingLot))
    pl = ParkingLot.from_buffer_copy(buff)
    print(buff)
    print(pl)
    print("주차장 번호 : ", pl.number)
    print("주차장 주소 : ", pl.address.decode('utf-8'))
    print("비어있는 자리 수 : ",pl.empty)
    print("총 주차 자리수 : ",pl.space)
    print("주차 배열 : ",pl.occupiedarea[:])

elif mess.message_id == 2 :
    buff = socket_client.recv(sizeof(Car))
    car = Car.from_buffer_copy(buff)
    print("주차장 번호 : ",car.parkingLotNumber)
    print("주차된 차량 위치 : ",car.occupiedNumber)
    print("차번호 :", car.carnumber.decode('utf-8'))


print("Closing socket")
socket_client.close()
    
