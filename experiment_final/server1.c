#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(){
int readfd,writefd;
 pid_t pid;
char buf[100];
 memset(buf,0,100);
int client1fd;
//创建管道
umask(0); //允许进程创建文件时有最大权限
 if(mkfifo("serverpipe",0644)<0) //创建命名管道
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
writefd = open("serverpipe",O_WRONLY,0);
  char str[100] = {0};   //keyboard input
    //scanf("%s",str);

    fgets(str,100,stdin);
    str[strlen(str)-1]='\0';
write(writefd,str,strlen(str)+1);
			printf("Message from keyboard input:%s\n", str);
}
 }else{
//创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(1234);  //端口
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);


    printf("waiting client...\n");
    //接收客户端请求
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(-1==clnt_sock)
    {
	    printf("error\n");
	    return 2;
    }
    printf("connectc client success..\n");




while(1)
{readfd = open("serverpipe",O_RDONLY,0);
 read(readfd,buf,100);
//printf("Message from server:%s\n", buf);
    write(clnt_sock, buf, strlen(buf)+1);

        printf("waiting  repeat\n");

    int r;
    char recvMsg[100] = {0};
	r = recv(clnt_sock, recvMsg, 255, NULL);
	if (r > 0) {
			printf("Message from client:%s\n", recvMsg);
    }
			
		}


 }
 }










 
