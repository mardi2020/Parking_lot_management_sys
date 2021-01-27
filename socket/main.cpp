#include "include/message.h"
#include "include/Car.h"
#include "include/Parkinglot.h"
#include "include/parkinglotinfo.h"
#include "include/carinfo.h"
#include <iostream>
//#include <time.h> //차량 등록시 시간 추가
#include <cstring>
#include <csignal>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h> // 시스템 함수
#include <pthread.h>
//#include <signal.h>
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
char* ParsingData(char*buff, char*str);
void *thread_function1(void *data);
void *thread_function2(void *data);
void *thread_function3(void *data);
void *thread_function4(void *data);

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
    struct sockaddr_in server_addr, client_addr;

    MYSQL Conn; // mysql 정보를 담을 구조체
    MYSQL_RES* res; // 쿼리 결과를 담는 구조체 포인터
    MYSQL_ROW row; // 쿼리 결과의 행을 담는 구조체
    signal( SIGPIPE, SIG_IGN );
    signal(SIGINT, signalHandler); // 컨트롤 C로 끌 때 소켓 종료

    mysql_init(&Conn); 
    mysql_options(&Conn, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(&Conn, MYSQL_INIT_COMMAND, "SET NAMES utf8");
    ConnPtr = mysql_real_connect(&Conn, HOST, USER, PASSWORD, DATABASE, PORT, (char*)NULL, 0);

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return 1;
    
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // bind(소켓 디스크립터, IP, 주소 길이)
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
            std::cout<<"accept() error"<<endl;
        else{
            int recv_ = recv(client_fd, (message_format *)&mess, sizeof(mess), 0);

            std::cout<<"받은 메시지 : "<<mess.message_id<<", "<<mess.data<<", "<<mess.len<<endl;

            if(1 == mess.message_id){ // 주차장 정보 조회
                pthread_create(&thread_id, NULL, thread_function1, (void *)&client_fd);
            }
            else if (2 == mess.message_id){ // 차량 등록
                pthread_create(&thread_id, NULL, thread_function2, (void *)&client_fd);
            }
            else if(3 == mess.message_id){ 
                pthread_create(&thread_id, NULL, thread_function3, (void *)&client_fd);
            }
            else if(4 == mess.message_id){ // 차량 요금 정산
                pthread_create(&thread_id, NULL, thread_function4, (void *)&client_fd);
            }
        }
    }
}

void *thread_function1(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    ParkingLot pl;
    std::string id = std::string(mess.data);
    Parkinglot PL(id, ConnPtr); 
    pl.number = atoi(mess.data);
    strcpy(pl.address, PL.Getaddress(id));
    //pl.address = PL.Getaddress(id); 
    pl.occupied = PL.GetOccupied(id);
    std::string str = PL.Getspace(id);
    std::stringstream ssInt(str);
    int i = 0; ssInt >> i;
    pl.space = i;
    PL.Setoccupiedarea(id);
    memcpy(pl.occupiedarea, PL.GetArray(), PL.GetSize());

    int s = (send(sockfd, (ParkingLot *)& pl, sizeof(pl), 0));
    if (s < 0)
        std::cout<<"send() error"<<endl;

    close(sockfd);
    std::cout<<"주차장 thread end"<<endl;
}
void *thread_function3(void *data){ // 정보 조회용
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    car c; // structure
    char id[11];
    memset(id, 0x00, sizeof(id));
    for(int i = 0; i < 10;i++)
        id[i] = mess.data[i];
    
    Car CAR(ConnPtr, id); // class
    strcpy(c.carnumber, CAR.GetCarnum());
    CAR.SetOccupiedNum();
    CAR.SetParkingNum();
    c.occupiedNumber = CAR.GetOccupiedNum();
    c.parkingLotNumber =  CAR.GetParkingNum();
    send(sockfd, (car *)& c, sizeof(c), 0);
    close(sockfd);
    
    std::cout<<"차량 정보 thread end"<<endl;
}

void *thread_function2(void *data){ // 차량 등록
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    car c; // structure

    Parkinglot PL(std::to_string(mess.data[0]-48), ConnPtr);
    // 주차장 내 자리 정보
    int n = mess.data[0]-48; // 주차장 번호
    PL.Setoccupiedarea(std::to_string(mess.data[0]-48));
    int arr[10] = {0,};
    memcpy(arr, PL.GetArray(), PL.GetSize());
    send(sockfd, arr, sizeof(arr), 0);

    int recv_ = recv(client_fd, (message_format *)&mess, sizeof(mess), 0);
    std::cout<<mess.message_id<<' '<<mess.data<<' '<<mess.len<<endl;

    char id[11];
    memset(id, 0x00, sizeof(id));
    for(int i = 0; i < 10;i++)
        id[i] = mess.data[i];

    Car CAR(ConnPtr, id); // class
    CAR.SetParkinglotNum(n);
    strcpy(c.carnumber, CAR.GetCarnum());
    CAR.Setlocation(mess.len, mess);
    CAR.Print(); //
    CAR.InsertDataInDB();
    
    char buff[8] = "Success";
    (send(sockfd, buff, sizeof(buff), 0));
    close(sockfd);
    
    std::cout<<"차량 thread end"<<endl;
}

void *thread_function4(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    char id[11];
    memset(id, 0x00, sizeof(id));
    for(int i = 0; i < 10;i++)
        id[i] = mess.data[i];
    Car car(ConnPtr, id);
    car.SettleupExpense();
    char buff[101];
    sprintf(buff,"%ld", car.GetToll());
    int s = (send(sockfd, buff, sizeof(buff), 0));
    close(sockfd);
    
    std::cout<<"요금 thread end"<<endl;
}
