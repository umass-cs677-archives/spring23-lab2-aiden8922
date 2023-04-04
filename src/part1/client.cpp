#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<chrono>

#define PORT 8081
#define SERVER_IP "192.168.85.128"


void signal_handler(int sig);

int status, client_fd;

int main(int argc, char const* argv[])
{
    
    struct sockaddr_in serv_addr;
    const char* request = "trade BobCo 1";
    char buffer[1024] = { 0 };

    signal(SIGINT,signal_handler);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  
    int pid=getpid();
    for(;;){

        if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))< 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
        auto start = std::chrono::high_resolution_clock::now();
        send(client_fd, request, strlen(request), 0);
        read(client_fd, buffer, 1024);
        auto stop= std::chrono::high_resolution_clock::now();
        close(client_fd);
        printf("thread%d receives:%s time taken:%ld\n",pid, buffer,std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count());
        
    }

}
void signal_handler(int sig){
    close(client_fd);
    exit(0);
}