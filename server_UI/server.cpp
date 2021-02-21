#include "message.h"
#include "Car.h"
#include "Parkinglot.h"
#include "parkinglotinfo.h"
#include "carinfo.h"
#include "base64.h"
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
int client_cnt = 0;
int total_len = 0;
std::string base64_image = "";
socklen_t addrlen;
pthread_t thread_id;
sockaddr_in server_addr, client_addr;
MYSQL Conn;
MYSQL_RES* res;
MYSQL_ROW row;

pthread_t thread[5];
void *SendParkinglotInfo(void *data);
void *InsertCarData(void *data);
void *SendCarInfo(void *data);
void *SendExpense(void *data);
void EmptyMessage();
void *handle_message(void* arg);
void *connectSOCKET(void *data);
void connectMYSQL();
void signalHandler( int signum ) {
    std::cout << "Closing Sockets" << endl;
    close(listen_fd);
    close(client_fd);
    exit(signum);  
}

std::string Store_c(){
    std::vector<MYSQL_ROW> pv;
    std::string query = "";
    std::string str = "";
    query = "SELECT number, indate, intime, location_c, location_p FROM car WHERE location_c IS NOT NULL";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    int fields = mysql_num_fields(res);
    while((row = mysql_fetch_row(res)) != NULL){
        for(int i = 0 ; i < fields ; i++){
            str += std::string(row[i]);
            str += " ";
         }
         str += '\n';
     }
    mysql_free_result(res);
    query = "SELECT number, indate, intime, outdate, outtime, toll FROM car WHERE location_c IS NULL";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    fields = mysql_num_fields(res);
    while((row = mysql_fetch_row(res)) != NULL){
        for(int i = 0 ; i < fields ; i++){
            str += std::string(row[i]);
            str += " ";
         }
         str += '\n';
    }
    return str;
}
std::string Store_p(){
    std::vector<MYSQL_ROW> cv;
    std::string query = "select * from parkinglot";
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    int fields = mysql_num_fields(res);
    std::string str = "";
    while((row = mysql_fetch_row(res)) != NULL){
        for(int i = 0 ; i < fields ; i++){
            str += std::string(row[i]);
            str += " ";
        }
        str += '\n';
    }
    return str;
}


void CloseServer(){
    std::cout<< "Closing Sockets" << endl;
    close(listen_fd);
    close(client_fd);
    exit(0);
}

void server(){
    signal( SIGPIPE, SIG_IGN);
    signal(SIGINT, signalHandler);

    //connectMYSQL();
    pthread_create(&thread[0], NULL, connectSOCKET, NULL);
    sleep(3);
}

void SendParkinglotInfo(message_format mess, int sockfd){
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
    int arr[10] = {0,};
    memcpy(arr, PL.GetArray(), PL.GetSize());
    send(sockfd, arr, sizeof(arr), 0);
   // send(sockfd, (ParkingLot *)& pl, sizeof(pl), 0);
    std::cout<<"주차장 thread end"<<endl;
    EmptyMessage();
}
void SendCarInfo(message_format mess, int sockfd){ // 정보 조회용
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
    
    std::cout<<"차량 정보 thread end"<<endl;
    EmptyMessage();
}

void InsertCarData(message_format mess, int sockfd){ // 차량 등록

    // send recognized car number
    //char id[] = "22가 2222";
    //send(sockfd, id, sizeof(id), 0);

    // received request(2, parkinglot number, 1)
    message_format mess_pln;
    message_format mess_carnum;

    recv(sockfd, (message_format *)&mess_pln, sizeof(mess_pln), 0);
    recv(sockfd, (message_format *)&mess_carnum, sizeof(mess_carnum), 0);
    //recv(sockfd, (message_format *)&mess, sizeof(mess), 0);
    int n = atoi(mess_pln.data); // 주차장 번호

    //Parkinglot PL(std::to_string(n), ConnPtr);
    // send 주차장 내 자리 정보
    //PL.Setoccupiedarea(std::to_string(n));
    //int arr[10] = {0,};
    //memcpy(arr, PL.GetArray(), PL.GetSize());
    //send(sockfd, arr, sizeof(arr), 0);


    EmptyMessage();
    // received request(2, location number, 1)=
    int loc = atoi(mess.data); // jucha wichi


    Car CAR(ConnPtr, mess_carnum.data); // class
    CAR.SetParkinglotNum(n);
    CAR.Setlocation(loc);
    CAR.Print();
    CAR.InsertDataInDB();

    message_format mess_result;
    mess_result.message_id = 0;
    strcpy(mess_result.data, "Success");
    mess_result.len = strlen(mess_result.data);
    (send(sockfd, (message_format *)&mess_result, sizeof(message_format), 0));
    
    std::cout<<"차량 thread end"<<endl;
    EmptyMessage();
}

void SendExpense(message_format mess, int sockfd){
    socklen_t addrlen;
    sockaddr_in client_addr;
    addrlen = sizeof(client_addr);

    // 차량 번호 복사
    char id[12]; 
    //memcpy(id, mess.data, sizeof(mess.data));
    strcpy(id, mess.data);
    // 클라이언트로부터 받은 차량 번호로 객체 생성
    Car car(ConnPtr, id);//std::vector<MYSQL_ROW> v1;
    //std::vector<MYSQL_ROW> v2;
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

struct tmp{
    int client_fd;
    int mess_len;
};



void *RecvPhoto(void *data){
    struct tmp t = *((tmp *) data);
    int sock_fd = t.client_fd;
    int len = t.mess_len;
    socklen_t addrlen;
    sockaddr_in client_addr;
    addrlen = sizeof(client_addr);

    base64_image += mess.data;

    // decode base64 image
    if(base64_image.size() == total_len){
        unsigned char* dout = (unsigned char*)malloc(BASE64_DECODE_OUT_SIZE(total_len));
        base64_decode(base64_image.c_str(), base64_image.size(), dout);

    }

}


void *handle_message(void* arg) {
    int client_fd = (int) *((int*) arg);
    std::cout << "Client Connected!\n";
    while(true){
        int len = recv(client_fd, (message_format *)&mess, sizeof(mess), 0);
        if (len<0){
            std::cout << strerror(errno) << endl;
            close(client_fd);
            break;
         }
        std::cout<<"message : "<<mess.message_id<<", "<<mess.data<<", "<<mess.len<<endl;

        if(1 == mess.message_id){
            SendParkinglotInfo(mess, client_fd);
        }
        else if(2 == mess.message_id){
            InsertCarData(mess, client_fd);
        }
        else if(3 == mess.message_id){
            SendCarInfo(mess, client_fd);
        }
        else if(4 == mess.message_id){
            SendExpense(mess, client_fd);
        }
        else if(5 == mess.message_id){
            tmp t;
            t.client_fd = client_fd;
            t.mess_len = mess.len;
            pthread_create(&thread[4], NULL, RecvPhoto, (void *)&t);
            pthread_join(thread[4], NULL);

        }
        else if(6 == mess.message_id){
            total_len = mess.len;
        }
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
    mysql_options(&Conn, MYSQL_OPT_LOCAL_INFILE, 0);
	ConnPtr = mysql_real_connect(&Conn, HOST,USER, PASSWORD, DATABASE, PORT, (char*)NULL, 0);
}

std::string WriteData(std::string FILE){
    std::string query = "SELECT * INTO OUTFILE '"+FILE+"'"+" FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\\n' FROM parkinglot";
    mysql_query(ConnPtr, query.c_str());
    return query;
}

std::string ReadData(std::string FILE){
    std::string query = "LOAD DATA LOCAL INFILE '" + FILE + "' "+"INTO TABLE parkinglot CHARACTER SET UTF8 FIELDS TERMINATED BY ',';";
    mysql_query(ConnPtr, query.c_str());

    std::cout << mysql_error(ConnPtr) << '\n';
    return query;
}

std::string SearchData(std::string query){
    MYSQL_RES *res;
    MYSQL_ROW row;
    mysql_query(ConnPtr, query.c_str());
    res = mysql_store_result(ConnPtr);
    int count = mysql_num_rows(res);
    std::string str = "";
    if(count > 0){
        int fields = mysql_num_fields(res);
        row = mysql_fetch_row(res);
        for(int i = 0 ; i < fields ; i++){
            str += std::string(row[i]);
            str += " ";
        }
    }
    return str;

}
bool Isout(std::string keyword, std::string table){
    std::string query = "";
    if(table == "car")
        query = "select location_c from "+table+" where number = '"+keyword+"'";
    else if(table == "parkinglot")
        query = "select location_c from "+table+" where number = "+keyword;

    mysql_query(ConnPtr, query.c_str());
    MYSQL_RES* res = mysql_store_result(ConnPtr);
    int num = mysql_num_rows(res);
    if (num <= 1){
        return true;
    }
    return false;
}
