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

 int fd_file0,fd_file1,fd_scull0,fd_scull1;
 int data;
 char buf0[30];//写入scull设备的内容
 char buf1[30];
 char buf_read0[4096]; //scull设备的内容读入到该buf中
 char buf_read1[4096];
 
  if((fd_file0 = open("/mnt/user1/scull0data",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))<0){
  perror("open");
  printf("open FILE0 WRONG！\n");
  exit(1);
  }
 else
  printf("open FILE0 SUCCESS!\n");
  
  if((fd_file1 = open("/mnt/user2/scull1data",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))<0){
  perror("open");
  printf("open FILE1 WRONG！\n");
  exit(1);
  }
 else
  printf("open FILE1 SUCCESS!\n");
  
  read(fd_file0, buf0, 30);
  
  read(fd_file1, buf1, 30);
  
 if((fd_scull0 = open("/dev/scull0",O_RDWR))<0){
 //打开scull设备
  perror("open");
  printf("open scull0 WRONG！\n");
  exit(1);
  }
 else
  printf("open scull0 SUCCESS!\n");

  
  if(write(fd_scull0,buf0,sizeof(buf0) ) == 0)
    printf("write failed!\n");
  
printf("buf0 is :%s\n",buf0); 

 if((fd_scull1 = open("/dev/scull1",O_RDWR))<0){
 //打开scull设备
  perror("open");
  printf("open scull1 WRONG！\n");
  exit(1);
  }
 else
  printf("open scull1 SUCCESS!\n");

  
  if(write(fd_scull1,buf1,sizeof(buf1) ) == 0)
    printf("write failed!\n");
  
printf("buf1 is :%s\n",buf1); 

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

      write(client1fd,recvMsg,strlen(recvMsg)+1);//写到子进程
			printf("Message from server:%s\n", recvMsg);
			
		}
    if(recvMsg[0] == 'r' && recvMsg[1] == 'e' && recvMsg[2] == 'a' && recvMsg[3] == 'd' && recvMsg[4] == '0'){

    printf("send message: scull0 data\n");
    lseek(fd_scull0,0,SEEK_SET); //把文件指针重新定位到文件开始的位置
 
    read(fd_scull0,buf_read0,strlen(buf0)); //把scull设备中的内容读入到buf_read中
    write(sock, buf_read0, strlen(buf_read0));
    //recvMsg = "scull control";
    //write(sock, recvMsg, strlen(recvMsg));
    }else if(recvMsg[0] == 'r' && recvMsg[1] == 'e' && recvMsg[2] == 'a' && recvMsg[3] == 'd' && recvMsg[4] == '1'){

    printf("send message: scull1 data\n");
    lseek(fd_scull1,0,SEEK_SET); //把文件指针重新定位到文件开始的位置
 
    read(fd_scull1,buf_read1,strlen(buf1)); //把scull设备中的内容读入到buf_read中
    write(sock, buf_read1, strlen(buf_read1));
    //recvMsg = "scull control";
    //write(sock, recvMsg, strlen(recvMsg));
    }else
    {
        printf("error instruction!\n");
        char error[] = "error instruction!";
        write(sock, error, strlen(error));
    }


 }
 }
}



