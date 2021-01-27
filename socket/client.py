from socket import *
from ctypes import *

TOTAL = 10
PORT = 12345 

class Car(Structure) :
    _fields_ = [
        ("parkingLotNumber", c_int),
        ("occupiedNumber", c_int),
        ("carnumber", c_char * 100),
        ("startdate", c_char * 11),
        ("starttime", c_char * 9),
    ]

class message_format(Structure) :
    _fields_ = [
        ("message_id", c_short),
        ("sub_id", c_short),
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

class array(Structure) :
    _fields_ = [
        ("arr", c_int * 10)
    ]

print("1. 주차장 정보 요청")
print("2. 차량 등록") 
print("3. 차량 정보 요청")
print("4. 차량 요금 정산") # 나갈때
command = int(input("  >> "))

socket_client = socket(AF_INET, SOCK_STREAM)
socket_client.connect(('127.0.0.1', PORT))
mess = message_format()
if command == 1 : # 조회
    mess.message_id = 1
    data = input("  주차장 번호 입력(1~5) : ")
    mess.len = len(data)
    mess.data = bytes(data + '\0', 'utf-8')

elif command == 2 :
    mess.message_id = 2
    data = input("  주차장 번호 :")
    mess.len = len(data)
    mess.data = bytes(data + '\0', 'utf-8')
    socket_client.send(mess)

    buff = socket_client.recv(sizeof(array))
    a = array.from_buffer_copy(buff)
    print("주차장 자리 : ", a.arr[:])

    mess.message_id = 2
    data = input("  차량 번호_주차자리 : ")# 추후 차량 번호판 인식으로 변경
    mess.len = len(data)
    mess.data = bytes(data + '\0', 'utf-8')

elif command == 3 :
    mess.message_id = 3
    data = input("  차번호 : ")
    mess.len =  len(data)
    mess.data = bytes(data + '\0', 'utf-8')

elif command == 4 :
    mess.message_id = 4
    data = input("  차량 번호 입력(11가 1111) : ")# 추후 차량 번호판 인식으로 변경
    mess.len = len(data)
    mess.data = bytes(data + '\0', 'utf-8')


socket_client.send(mess)


if mess.message_id == 1 and command == 1:
    buff = socket_client.recv(sizeof(ParkingLot))
    pl = ParkingLot.from_buffer_copy(buff)
    print("주차장 번호 : ", pl.number)
    print("주차장 주소 : ", pl.address.decode('utf-8'))
    print("비어있는 자리 수 : ",pl.space - pl.empty)
    print("총 주차 자리수 : ",pl.space)
    print("주차 배열 : ",pl.occupiedarea[:])

elif command == 2 :
    buff = socket_client.recv(8)
    print(buff.decode())

elif mess.message_id == 3 and command == 3:
    buff = socket_client.recv(sizeof(Car))
    car = Car.from_buffer_copy(buff)
    print("주차장 번호 : ",car.parkingLotNumber)
    print("주차된 차량 위치 : ",car.occupiedNumber)
    print("차번호 :", car.carnumber.decode('utf-8'))

elif command == 4 :
    data = socket_client.recv(100)
    print("요금 : ", data.decode())
    
print("Closing socket")
socket_client.close()
    
