#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // 소켓 지원을 위한 각종 함수
#include <cstring>
#include <unistd.h> // 시스템 함수

#include <pthread.h>
#include <signal.h>

#define endl '\n'
#define MAX 1024
#define PORT_NUM 12345
#define TOTAL 10

int Sockfd;
struct ParkingLot{
    int parkingLotNumber;
    int occupiedNumber;
    int totalNumber = TOTAL;
    int occupiedarea[TOTAL] = {0,};
};

void messagefunc(ParkingLot &pl, int x, int y, int z, int* arr){
    pl.parkingLotNumber = x;
    pl.occupiedNumber = y;
    pl.totalNumber = z;
    memcpy(pl.occupiedarea, arr, sizeof(int)*TOTAL);
}


void *thread_function(void *data){
    int sockfd = *((int *) data);
    socklen_t addrlen;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);

    ParkingLot PL;
    int arr[TOTAL] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    messagefunc(PL, 1, 2, TOTAL, arr);
    int s = (send(sockfd, (ParkingLot *)& PL, sizeof(PL), 0));

    close(sockfd);

    std::cout<<"Client thread end"<<endl;
}


int main(){
    int listen_fd, client_fd;
    socklen_t addrlen;
    pthread_t thread_id;
    struct sockaddr_in server_addr, client_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return 1;
    
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PORT_NUM;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl : IP는 32비트이므로
    // INADDR_ANY : 컴퓨터에 존재하는 랜카드 중 사용 가능한 랜카드의 ip 주소 사용
    
    // bind(소켓 디스크립터, IP, 주소 길이)
    if(::bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind() error");
        return 1;
    }
    
    std::cout<<"Server is started at port "<<PORT_NUM<<endl;

    // 요청이 들어오면 수락할 수 있는 대기상태
    // listen(소켓 디스크립터, 연결 대기열의 크기)
    if(listen(listen_fd, 5) < 0){
        perror("listen() error");
        return 1;
    }

    while(1){
        addrlen = sizeof(client_addr);
        // 서버 소켓에 클라이언트를 연결하는 함수
        // accept(소켓 디스클비터, 클라이언트 주소 정보의 구조체, 2번째 인자값의 길이)
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
        if(client_fd < 0)
            std::cout<<"accept() error"<<endl;
        else{
            // pthread_create(스레드의 id 저장을 위한 변수의 주소 전달, 
            // 스레드에 부여할 특성 정보의 전달을 위한 매개 변수[NULL : 기본적인 특성의 스레드 생성],
            // 스레드의 메인 함수 역할을 하는 별도 실행 흐름의 시작이 되는 함수의 주소 값 전달,
            // 세 번째 인자를 통해 등록된 함수가 호출될 때 전달할 인자의 정보를 담고 있는 변수의 주소 전달)
            
            pthread_create(&thread_id, NULL, thread_function, (void *)&client_fd);

            pthread_detach(thread_id);
        }
    }
}
