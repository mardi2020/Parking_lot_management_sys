#include "include/message.h"
#include "include/Car.h"
#include "include/Parkinglot.h"
#include "include/parkinglotinfo.h"
#include "include/carinfo.h"

#include <iostream>
#include <time.h> //차량 등록시 시간 추가
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h> // 시스템 함수
#include <pthread.h>
#include <signal.h>
#include </usr/include/mariadb/mysql.h>

#define endl '\n'
#define PORT_NUM 12345
#define TOTAL 10
// mariaDB
#define HOST "localhost"
#define USER "root"
#define PASSWORD "test"
#define DATABASE "parking"
#define PORT 3306

MYSQL* ConnPtr = nullptr; // mysql
message_format mess;
std::string ParsingData();
void *thread_function1(void *data);
void *thread_function2(void *data);
void *thread_function3(void *data);

int main(){
    int listen_fd, client_fd;
    socklen_t addrlen;
    pthread_t thread_id;
    struct sockaddr_in server_addr, client_addr;

    MYSQL Conn; // mysql 정보를 담을 구조체
    MYSQL_RES* res; // 쿼리 결과를 담는 구조체 포인터
    MYSQL_ROW row; // 쿼리 결과의 행을 담는 구조체

    mysql_init(&Conn); 
    mysql_options(&Conn, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(&Conn, MYSQL_INIT_COMMAND, "SET NAMES utf8");

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
            else if(3 == mess.message_id){ // 차량 요금 정산
                pthread_create(&thread_id, NULL, thread_function3, (void *)&client_fd);
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
    Parkinglot PL(std::string(mess.data), ConnPtr);
    pl.number = atoi(mess.data);
    pl.address = PL.Getaddress(std::string(mess.data));
    pl.occupied = PL.GetOccupied(std::string(mess.data));
    std::string str = PL.Getspace(std::string(mess.data));
    std::stringstream ssInt(str);
    int i = 0; ssInt >> i;
    pl.space = i;
    memcpy(pl.occupiedarea, PL.GetArray(), sizeof(PL.GetArray()));

    int s = (send(sockfd, (ParkingLot *)& pl, sizeof(pl), 0));
    if (s < 0)
        std::cout<<"send() error"<<endl;

    close(sockfd);
    std::cout<<"주차장 thread end"<<endl;
}
std::string ParsingData(){
    std::string str = "";
    for (int i = 0; i < mess.len;i++) {
        if(0 <= i && i < 8) // 차번호
               str[i] = mess.data[i];
    }
    return str;
}
void *thread_function2(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    car c; // structure
    Car CAR(ConnPtr, (char *)ParsingData().c_str()); // class
    strcpy(c.carnumber, CAR.GetCarnum());
    CAR.Setlocation(mess.len, mess);
    c.occupiedNumber = CAR.GetOccupiedNum();
    c.parkingLotNumber =  CAR.GetParkingNum();
    (send(sockfd, (car *)& c, sizeof(c), 0));
    close(sockfd);
    std::cout<<"차량 thread end"<<endl;
}

void *thread_function3(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    Car car(ConnPtr, (char *)ParsingData().c_str());
    char buff[100];
    sprintf(buff,"%ld", car.GetToll());
    int s = (send(sockfd, buff, sizeof(buff), 0));
    close(sockfd);
    std::cout<<"요금 thread end"<<endl;
}