#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <error.h>
#include <sys/ioctl.h>

//定义幻数
#define SCULL_IOC_MAGIC '$'

//定义命令->
//数据清零
#define SCULL_IOC_CLEAR _IO(SCULL_IOC_MAGIC, 0)
//获取数据(通过指针)
#define SCULL_IOC_GET   _IOR(SCULL_IOC_MAGIC, 1, int)
//获取数据(通过返回值)
#define SCULL_IOC_QUERY _IO(SCULL_IOC_MAGIC, 2)
//设置数据(通过指针)
#define SCULL_IOC_SET   _IOW(SCULL_IOC_MAGIC, 3, int)
//设置数据(通过直接引用参数值)
#define SCULL_IOC_TELL  _IO(SCULL_IOC_MAGIC, 4)


int main()
{
 int fd;
 int data;
 char buf[]="scull character device test by liweiheng";//写入scull设备的内容
 char buf_read[4096]; //scull设备的内容读入到该buf中
 
 if((fd=open("/dev/scull1",O_RDWR))<0){
 //打开scull设备
  perror("open");
  printf("open scull WRONG！\n");
  exit(1);
  }
 else
  printf("open scull SUCCESS!\n");
  
 printf("buf is :%s\n",buf); 
 write(fd,buf,sizeof(buf)); //把buf中的内容写入scull设备
 
 lseek(fd,0,SEEK_SET); //把文件指针重新定位到文件开始的位置
 
 read(fd,buf_read,sizeof(buf)); //把scull设备中的内容读入到buf_read中
 
  printf("buf_read is :%s\n",buf_read);

//数据清零  
    ioctl(fd, SCULL_IOC_CLEAR);

//直接传值测试
    data = ioctl(fd, SCULL_IOC_QUERY);
    data = 100;
    ioctl(fd, SCULL_IOC_TELL, data);

//指针传值测试
    ioctl(fd, SCULL_IOC_GET, &data);
    data = 122;
    ioctl(fd, SCULL_IOC_SET, &data);
 

 
 return 0;
}

