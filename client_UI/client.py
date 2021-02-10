from socket import *
from struct import *
from message_format import *
from array import *
from parkinglot import *
from Car import *
import base64

ID = 1
PORT = 12345

class Application :
    def __init__(self):
        self.socket_client = socket(AF_INET, SOCK_STREAM)
        self.socket_client.connect(('127.0.0.1', PORT))
        

    def RequestPLinfo(self):
        mess = message_format()
        mess.message_id = 1
        data = ID
        mess.len = len(ID)
        mess.data = bytes(data +'\0', 'utf-8')

        self.socket_client.send(mess)

    def RecvPLinfo(self):
        buff = self.socket_client.recv(sizeof(ParkingLot))
        pl = ParkingLot.from_buffer_copy(buff)
        num = pl.number
        address = pl.address.decode('utf-8')
        space = pl.space - pl.empty
        total = pl.space
        arr = pl.occupiedarea[:]

        return pl, num, address, space, total, arr

    def Requestspace(self):
        mess = message_format()
        mess.message_id = 2
        data = ID
        mess.len = len(ID)
        mess.data = bytes(data + '\0', 'utf-8')
        self.socket_client.send(mess)

        buff = self.socket_client.recv(sizeof(array))
        arr = array.from_buffer_copy(buff)
        space = arr.arr[:]

        return space

    def AddCar(self):
        mess = message_format()
        mess.message_id = 2
        data = "222가 2222" # 나중에 인식되어 나온 값으로 변경
        mess.len = len(data)
        mess.data = bytes(data + '\0', 'utf-8')
        self.socket_client.send(mess)
    
    def Recvresult(self) :
        buff = self.socket_client.recv(8)
        
        return buff.decode() # 주차 성공결과

    def RequestCarinfo(self):
        mess = message_format()
        mess.message_id = 3
        data = "123가 1234" # 이것도 나중에 변경
        mess.len = len(data)
        mess.data = bytes(data + '\0', 'utf-8')
        self.socket_client.send(mess)

    def RecvCarinfo(self) :
        buff = self.socket_client.recv(sizeof(Car))
        car = Car.from_buffer_copy(buff)
        plnum = car.parkingLotNumber
        location = car.occupiedNumber
        carnum = car.carnumber.decode('utf-8')

        return car, plnum, location, carnum
    
    def SettleupExpense(self):
        mess = message_format() 
        mess.message_id = 4
        data = "11가 1111"
        mess.len = len(data)
        mess.data = bytes(data +'\0', 'utf-8')
        self.socket_client.send(mess)

    def Expense(self):
        data = self.socket_client.recv(100)
        expense = unpack('<I', data)[0] # 값이 튜플이므로

        return expense
    
    # local
    def SendToServer(self, path):
        mess = message_format()
        mess.message_id = 5
        data = path
        mess.len = len(data)
        mess.data = bytes(data + '\0', 'utf-8')
        self.socket_client.send(mess)

    # client to server( use base64 )
    def Send2Server(self, path):
        with open(path, "rb") as imageFile:
            str_ = base64.b64encode(imageFile.read())
        
        ll = len(str_)
        mess = message_format()
        mess.message_id = 5
        mess.len = ll
        data = 'encode base64'
        mess.data = bytes(data+'\0', 'utf-8')
        self.socket_client.send(mess)
        self.socket_client.close()

        for i in range(0, ll, 255) :
            self.socket_client = socket(AF_INET, SOCK_STREAM)
            self.socket_client.connect(('127.0.0.1', PORT))
            m = message_format()
            m.message_id = 5
            data = str_[i:i+255] + b'\0'
            m.len = len(data) - 1
            m.data = data
            self.socket_client.send(m)
            self.socket_client.close()