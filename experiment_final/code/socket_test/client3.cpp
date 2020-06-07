// LinuxClient.cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<iostream>
using namespace std;
int main()
{
 int readfd,writefd;
 pid_t pid;
char buf[100];
 memset(buf,0,100);
int client1fd;
;
//创建管道
umask(0); //允许进程创建文件时有最大权限
 if(mkfifo("client1pipe",0644)<0) //创建命名管道
 {
 perror("mkfifo"); //创建失败
 exit(1);
 }


 if((pid = fork()) == -1){
 perror("fork error");
 exit(EXIT_FAILURE);
 }else if(pid == 0){
 while(1)
{
buf[100] = {0};
readfd = open("client1pipe",O_RDONLY,0);
 read(readfd,buf,100);
 close(readfd);
 printf("read message from parents :%s\n", buf);
}
 }else{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
 
    //向服务器（特定的IP和端口）发起请求
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("192.168.0.108");  //具体的IP地址
    serv_addr.sin_port = htons(1234);  //端口
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

client1fd = open("client1pipe",O_WRONLY,0);//向cilent1发送信息

while(1)
{

    char recvMsg[100] = {0};
    int r;
		r = recv(sock, recvMsg, 255, NULL);
		if (r > 0) {
//fifo

write(client1fd,recvMsg,strlen(recvMsg)+1);
			printf("Message from server:%s\n", recvMsg);
			
		}
    write(sock, recvMsg, strlen(recvMsg));


 }
 }
}



