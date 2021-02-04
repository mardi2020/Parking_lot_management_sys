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
#include </usr/include/mysql/mysql.h>

#define endl '\n'
#define PORT_NUM 12345
#define TOTAL 10
#define HOST "localhost"
#define USER "root"
#define PASSWORD ""
#define DATABASE "parking"
#define PORT 3306
#define THREAD_NUM 5 //클라이언 동시 접속 수

MYSQL* ConnPtr; 
message_format mess;
int listen_fd, client_fd;
socklen_t addrlen;
pthread_t thread_id;
sockaddr_in server_addr, client_addr;
MYSQL Conn;
MYSQL_RES* res;
MYSQL_ROW row;
std::vector<MYSQL_ROW> pv;
std::vector<MYSQL_ROW> cv;
pthread_t thread[5]; 
/*--thread function--*/
void *SendParkinglotInfo(void *data);
void *InsertCarData(void *data);
void *SendCarInfo(void *data);
void *SendExpense(void *data);
void EmptyMessage();

void signalHandler( int signum ) {
    std::cout << "Closing Sockets" << endl;
    close(listen_fd);
    close(client_fd);
    exit(signum);  
}

int client_cnt = 0;

void *handle_message(void* arg) {
    int client_fd = (int) *((int*) arg);
    recv(client_fd, (message_format *)&mess, sizeof(mess), 0);

	std::cout<<"message : "<<mess.message_id<<", "<<mess.data<<", "<<mess.len<<endl;

	if(1 == mess.message_id){
		pthread_create(&thread[1], NULL, SendParkinglotInfo, (void *)&client_fd);
        pthread_join(thread[1], NULL);
    }
	else if(2 == mess.message_id){
		pthread_create(&thread[2], NULL, InsertCarData, (void *)&client_fd);
        pthread_join(thread[2], NULL);
    }
	else if(3 == mess.message_id){
		pthread_create(&thread[3], NULL, SendCarInfo, (void *)&client_fd);
        pthread_join(thread[3], NULL);
    }
	else if(4 == mess.message_id){
		pthread_create(&thread[4], NULL, SendExpense, (void *)&client_fd);
        pthread_join(thread[4], NULL);
    }
}

void *connectSOCKET(void *data){
    pthread_t client_tid[THREAD_NUM];
    int client_fds[THREAD_NUM];
    int status;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(1);
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(::bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		exit(1);
	
	std::cout<<"Server is started at port "<<PORT_NUM<<"."<<endl;

	while(1){
        if(listen(listen_fd, 5) < 0)
		    exit(1);
		addrlen = sizeof(client_addr);
		client_fds[client_cnt] = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

		if(client_fds[client_cnt] < 0)
			exit(1);
		else{
            if ((status = pthread_create(&client_tid[client_cnt], NULL, &handle_message, (void *)&client_fds[client_cnt])) != 0) {
                exit(1);
            }
            pthread_join(client_tid[client_cnt], NULL);
            client_cnt++;
            if (client_cnt == THREAD_NUM)
                client_cnt = 0;
		}
		pthread_detach(thread_id);
	}
	mysql_free_result(res);
	mysql_close(ConnPtr);
}

void connectMYSQL(){
	mysql_init(&Conn);
	mysql_options(&Conn, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(&Conn, MYSQL_INIT_COMMAND, "SET NAMES utf8");
	ConnPtr = mysql_real_connect(&Conn, HOST,USER, PASSWORD, DATABASE, PORT, (char*)NULL, 0);
}


int main(){ 
    signal( SIGPIPE, SIG_IGN);
    signal(SIGINT, signalHandler);
    connectMYSQL();
    pthread_create(&thread[0], NULL, connectSOCKET, NULL);
    std::cout<<"wait 3 seconds..\n";
    sleep(3);
    while(1){

    	std::cout<<"   <<";
    	int command = 0; std::cin>>command;
    	// Send info to client
    	if(command == 1){ // search data - parkinglot
    		std::string query = "select * from car";
    		mysql_query(ConnPtr, query.c_str());
    		res = mysql_store_result(ConnPtr);
    		while((row = mysql_fetch_row(res)) != NULL)
    			pv.push_back(row);

            for(auto e: pv)
                std::cout<<e[0]<<e[1]<<endl;
    	}
    	else if(command == 2){
		    std::string query = "select * from parkinglot";
    		mysql_query(ConnPtr, query.c_str());
    		res = mysql_store_result(ConnPtr);
    		while((row = mysql_fetch_row(res)) != NULL)
    			cv.push_back(row);
            
            for(auto e: cv)
                std::cout<<e[0]<<e[1]<<endl;
    	}
    	else if(command == 3){
            std::cout<<"** Good bye **"<<endl;
    		std::cout << "Closing Sockets" << endl;
    		close(listen_fd);
    		close(client_fd);
    		return 0;
    	}

    }
	
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
