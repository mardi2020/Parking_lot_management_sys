#include <iostream>
#include <time.h> //차량 등록시 시간 추가
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // 소켓 지원을 위한 각종 함수
#include <unistd.h> // 시스템 함수
#include <pthread.h>
#include <signal.h>
#include </usr/include/mariadb/mysql.h>
#include <iomanip>
#include <vector>
#include <sstream>
#define endl '\n'
#define PORT_NUM 12345
#define TOTAL 10
// mariaDB
#define HOST "localhost"
#define USER "root"
#define PASSWORD "test"
#define DATABASE "parking"
#define PORT 3306
#define TABLENAME "car"

struct Car{
    int parkingLotNumber;
    int occupiedNumber;
    char carnumber[100] = {0,};
    //int totalNumber = TOTAL;
    //int occupiedarea[TOTAL] = {0,};
};
struct ParkingLot{
    int number; // 주차장의 번호
    char address[256]; // 주차장 주소
    int empty; // 주차장의 빈 자리
    int space; // 주차장 총 주차 자리
    int occupiedarea[TOTAL] = {0,};
};
std::vector<MYSQL_ROW> car_vector; // 차량 정보 저장
std::vector<MYSQL_ROW> parkinglot_vector; // 주차장정보 저장
int mess_id;
char mess_data[256];

struct message_format {
    short message_id; // tag
    int len; // length
    char data[256]; // data
};

void sig_handler(int signo);
void ComputeOccupiedArea(std::vector<MYSQL_ROW> row, int arr[]);
void setmessagecar(Car &car, int x, int y, char num[]);
void setmessageparkinglot(ParkingLot &pl, int num, char add[], int emt, int space, int ary[]);
void *thread_function1(void *data);
void *thread_function2(void *data);
MYSQL_RES* UpdateOccupiedNumber(MYSQL* Connptr, int parkinglotnum);

int main(){
    int listen_fd, client_fd;
    socklen_t addrlen;
    pthread_t thread_id;
    struct sockaddr_in server_addr, client_addr;

    MYSQL Conn; // mysql 정보를 담을 구조체
    MYSQL* ConnPtr = nullptr; // mysql
    MYSQL_RES* res; // 쿼리 결과를 담는 구조체 포인터
    MYSQL_ROW row; // 쿼리 결과의 행을 담는 구조체
    int status; // 쿼리 요청 후 결과

    /* mysql 연결 */
    mysql_init(&Conn);  // mysql 초기화
    // 한글 깨짐 설정
    mysql_options(&Conn, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(&Conn, MYSQL_INIT_COMMAND, "SET NAMES utf8");

    ConnPtr = mysql_real_connect(&Conn, HOST, USER, PASSWORD, DATABASE, PORT, (char*)NULL, 0);
    
    if (ConnPtr == nullptr){
        perror("connection error");
        return 1;
    }

    mysql_query(ConnPtr, "SELECT * FROM car");
    res = mysql_store_result(ConnPtr);
    while((row = mysql_fetch_row(res)) != NULL){
        car_vector.push_back(row);
    }
    res = UpdateOccupiedNumber(ConnPtr, 1);

    /* socket 연결 */
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return 1;
    
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl : IP는 32비트이므로
    
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
            message_format message;
            int recv_ = recv(client_fd, (message_format *)&message, sizeof(message), 0);

            if(recv_ > 0){
                std::cout<<"받은 메시지 : "<<message.message_id<<", "<<message.data<<", "<<message.len<<endl;
            }
            mess_id = message.message_id;
            strcpy(mess_data, message.data);

            if(1 == message.message_id){   // 주차장
                mysql_query(ConnPtr, "SELECT * FROM parkinglot");
                res = mysql_store_result(ConnPtr);
                while((row = mysql_fetch_row(res)) != NULL){
                    parkinglot_vector.push_back(row);
                }
                pthread_create(&thread_id, NULL, thread_function1, (void *)&client_fd);
            }
            else if (2 == message.message_id){ // 차량
                pthread_create(&thread_id, NULL, thread_function2, (void *)&client_fd);
            }

            memset(message.data, 0x00, sizeof(message.data));
            pthread_detach(thread_id);
        }
    }
    
    mysql_free_result(res);
}

void ComputeOccupiedArea(std::vector<MYSQL_ROW> row, int arr[]){
    int n = row.size();
    for(int i = 0 ;i < n ;i++){
        if (row[i][5] != NULL)
            arr[atoi(row[i][5])-1] = 1;
    }
}
void setmessagecar(Car &car, int x, int y, char num[]){
    car.parkingLotNumber = x;
    car.occupiedNumber = y;
    strcpy(car.carnumber, num);
    //car.totalNumber = z;
    //memcpy(car.occupiedarea, arr, sizeof(int)*TOTAL);
}
void setmessageparkinglot(ParkingLot &pl, int num, char add[], int emt, int space, int ary[]){
    pl.number = num;
    strcpy(pl.address, add);
    pl.empty = emt;
    pl.space = space;
    memcpy(pl.occupiedarea, ary, sizeof(int)*TOTAL);
}
void *thread_function1(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    ParkingLot pl;

    int arr[TOTAL] = {0,}; // 자동차가 주차된 위치 저장하는 배열
    ComputeOccupiedArea(car_vector, arr); 
    int num = 0; // 이미 주차된 자리 수
    for(auto e : car_vector){
        if(e[5] != NULL)
            num += 1;
    }

    setmessageparkinglot(pl, atoi(mess_data), parkinglot_vector[atoi(mess_data)][1], TOTAL-num, TOTAL, arr);
    int s = (send(sockfd, (ParkingLot *)& pl, sizeof(pl), 0));
    if (s < 0)
        std::cout<<"send() error"<<endl;

    close(sockfd);
    std::cout<<"주차장 thread end"<<endl;
}

void *thread_function2(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    Car car;

    int parkinglotnum = 0, occupiednum = 0;
    for (auto e : car_vector){
        if (strcmp(e[0], mess_data) == 0){
            if(e[5] != NULL && e[6] != NULL){
                parkinglotnum = atoi(e[6]);
                occupiednum = atoi(e[5]);
            }
        }
    }
    
    setmessagecar(car, parkinglotnum, occupiednum, mess_data);
    int s = (send(sockfd, (Car *)& car, sizeof(car), 0));

    close(sockfd);
    std::cout<<"차량 thread end"<<endl;
}

// Car 테이블의 location_p 변수를 count하여 parkinglot 테이블 occupied에 반영
MYSQL_RES* UpdateOccupiedNumber(MYSQL* Connptr, int parkinglotnum){
    std::string String = static_cast<std::ostringstream*>( &(std::ostringstream() << parkinglotnum))->str();
    MYSQL_RES* res;
    std::string query = "UPDATE parkinglot SET occupied=(SELECT COUNT(*) FROM car WHERE location_p = "+ String  +") WHERE number = "+ String;
    mysql_query(Connptr, query.c_str());
    res = mysql_store_result(Connptr);

    return res;
}

void sig_handler(int signo){
    
}