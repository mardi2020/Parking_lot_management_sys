#include "include/message.h"
#include "include/Car.h"
#include "include/Parkinglot.h"
#include "include/parkinglotinfo.h"
#include "include/carinfo.h"
#include <iostream>
#include <cstring>
#include <csignal>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h>
#include </usr/include/mariadb/mysql.h>

#define endl '\n'
#define PORT_NUM 12345
#define TOTAL 10
#define HOST "localhost"
#define USER "root"
#define PASSWORD "test"
#define DATABASE "parking"
#define PORT 3306

MYSQL* ConnPtr; 
message_format mess;
int listen_fd, client_fd;
/*--thread function--*/
void *SendParkinglotInfo(void *data);
void *InsertCarData(void *data);
void *SendCarInfo(void *data);
void *SendExpense(void *data);
void EmptyMessage();

// ctrl+ c 시그널을 잡아 socket 닫기
void signalHandler( int signum ) {
    std::cout << "Closing Sockets" << endl;
    close(listen_fd);
    close(client_fd);
    exit(signum);  
}

int main(){
    
    socklen_t addrlen;
    pthread_t thread_id;
    sockaddr_in server_addr, client_addr;

    MYSQL Conn; 
    MYSQL_RES* res; 
    MYSQL_ROW row; 
    signal( SIGPIPE, SIG_IGN );
    signal(SIGINT, signalHandler); // 컨트롤 C로 끌 때 소켓 종료

    mysql_init(&Conn); 
    mysql_options(&Conn, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(&Conn, MYSQL_INIT_COMMAND, "SET NAMES utf8");
    ConnPtr = mysql_real_connect(&Conn, HOST, USER, PASSWORD, DATABASE, PORT, (char*)NULL, 0);

    if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket() error");
        return 1;
    }
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(::bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind() error");
        return 1;
    }
    std::cout<<"Server is started at port "<<PORT_NUM<<endl;

    // listen(소켓 디스크립터, 연결 대기열의 크기)
    if(listen(listen_fd, 5) < 0){
        perror("listen() error");
        return 1;
    }

    while(1){
        addrlen = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

        if(client_fd < 0)
            perror("accept() error");
        else{
            int recv_ = recv(client_fd, (message_format *)&mess, sizeof(mess), 0);

            std::cout<<"받은 메시지 : "<<mess.message_id<<", "<<mess.data<<", "<<mess.len<<endl;

            if(1 == mess.message_id){ // 주차장 정보 조회
                pthread_create(&thread_id, NULL, SendParkinglotInfo, (void *)&client_fd);
            }
            else if (2 == mess.message_id){ // 차량 등록
                pthread_create(&thread_id, NULL, InsertCarData, (void *)&client_fd);
            }
            else if(3 == mess.message_id){ 
                pthread_create(&thread_id, NULL, SendCarInfo, (void *)&client_fd);
            }
            else if(4 == mess.message_id){ // 차량 요금 정산
                pthread_create(&thread_id, NULL, SendExpense, (void *)&client_fd);
            }
        }
        
        pthread_detach(thread_id);
    }

    mysql_free_result(res);
    mysql_close(ConnPtr);
}

void *SendParkinglotInfo(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    ParkingLot pl;
    std::string id = std::string(mess.data);
    Parkinglot PL(id, ConnPtr); 
    pl.number = atoi(mess.data);
    strcpy(pl.address, PL.Getaddress(id));
    pl.occupied = PL.GetOccupied(id);
    std::string str = PL.Getspace(id);
    std::stringstream ssInt(str);
    int i = 0; ssInt >> i;
    pl.space = i;
    PL.Setoccupiedarea(id); 
    memcpy(pl.occupiedarea, PL.GetArray(), PL.GetSize());

    send(sockfd, (ParkingLot *)& pl, sizeof(pl), 0);
    close(sockfd);
    std::cout<<"주차장 thread end"<<endl;
    EmptyMessage();
}
void *SendCarInfo(void *data){ // 정보 조회용
    int sockfd = *((int *) data);
    socklen_t addrlen;
    sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    car c; // structure
    char id[12];
    strcpy(id, mess.data);

    Car CAR(ConnPtr, id); // class
    strcpy(c.carnumber, CAR.GetCarnum());
    
    CAR.SetOccupiedNum(); 
    CAR.SetParkingNum();

    c.occupiedNumber = CAR.GetOccupiedNum();
    c.parkingLotNumber =  CAR.GetParkingNum();

    send(sockfd, (car *)& c, sizeof(c), 0);
    close(sockfd);
    
    std::cout<<"차량 정보 thread end"<<endl;
    EmptyMessage();
}

void *InsertCarData(void *data){ // 차량 등록
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);

    Parkinglot PL(std::to_string(mess.data[0]-48), ConnPtr);
    // 주차장 내 자리 정보
    int n = mess.data[0]-48; // 주차장 번호
    PL.Setoccupiedarea(std::to_string(mess.data[0]-48));
    int arr[10] = {0,};
    memcpy(arr, PL.GetArray(), PL.GetSize());
    
    send(sockfd, arr, sizeof(arr), 0);
    EmptyMessage();

    int recv_ = recv(client_fd, (message_format *)&mess, sizeof(mess), 0);
    std::cout<<mess.message_id<<' '<<mess.data<<' '<<mess.len<<endl;

    char id[12];
    memset(id, 0x00, sizeof(id));
    int i = 0;
    while(mess.data[i] != '_'){
        id[i] = mess.data[i];
        i++;
    }
    Car CAR(ConnPtr, id); // class
    CAR.SetParkinglotNum(n);
    CAR.Setlocation(mess.len, mess);
    CAR.Print(); //
    CAR.InsertDataInDB();
    
    char buff[8] = "Success";
    (send(sockfd, buff, sizeof(buff), 0));
    close(sockfd);
    
    std::cout<<"차량 thread end"<<endl;
    EmptyMessage();
}

void *SendExpense(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    sockaddr_in client_addr;
    addrlen = sizeof(client_addr);

    // 차량 번호 복사
    char id[12]; 
    //memcpy(id, mess.data, sizeof(mess.data));
    strcpy(id, mess.data);
    // 클라이언트로부터 받은 차량 번호로 객체 생성
    Car car(ConnPtr, id);
    car.SettleupExpense(); // 비용계산
    int expense = car.GetToll();
    std::cout<<expense<<endl;
    // Socket operation on non-socket  error 고치기
    int s = (send(sockfd, (int *)&expense, sizeof(expense), 0));
    if (s < 0)
        perror("send() error");
        
    close(sockfd);
    std::cout<<"요금 thread end"<<endl;
    EmptyMessage();
}


void EmptyMessage(){
    mess.message_id = 0;
    mess.len = 0;
    memset(mess.data, 0x00, sizeof(mess.data));
}