# **<center><font size=6>嵌入式软件开发技术与工具实验报告六</font></center>**

<center><font>黎炜桁</font></center>

---

### **一、实验目的**   

能够综合应用课程所学的技术与工具，包括：

- Socket通信
- 多进程、多线程编程
- 交叉调试目标端程序
- 磁盘分区与文件系统创建
- 模块与驱动编程

### **二、实验内容**   

1. 将树莓派设为智能家居Linux服务器，可用来采集并维护环境数据，如PM2.5、温度、湿度、气味、电器状态数据等。在实际环境中数据来自相应的传感器，本次试验中用scull设备模拟。有条件的小组鼓励使用真实设备采集数据。
2. 要求创建2个以上的scull设备，设备驱动可选择从内核源码树外(Kbuild)编译安装，或加入到内核源码树内。驱动函数要求包括：open, release, read, write, llseek, ioctl。
3. 实验中的环境数据存储在特定文件系统中。该文件系统要求具备属性：在线写入、持久性、断电可靠性。
4. PC机、移动设备或另外一个树莓派用作远程客户端，随时请求获取环境数据，客户端和服务器之间采用Socket通信。
5. APP编译采用交叉编译，用gdb-gdbserver交叉调试APP。

### **三、实验过程与结果**   

#### 1. scull设备驱动   

- scull设备创建   
本实验创建两个scull设备，函数包括：open, release, read, write, llseek, ioctl，实现对数据的操作。   
源代码分析：   

   - scull_open   

      ```c
      int scull_open(struct inode *inode, struct file *filp)
      {
       struct scull_dev *dev;
    
       dev = container_of(inode->i_cdev, struct scull_dev, cdev);
       filp->private_data = dev;
 
       if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
       {
           if (down_interruptible(&dev->sem))
           {
               return -ERESTARTSYS;
           }
           scull_trim(dev);
           up(&dev->sem);
       }
      ```

      scull_open函数实现打开一个scull设备


   - scull_release  

      ```C
       int scull_release(struct inode *inode, struct file *filp)
      {
          return 0;
      }
      ```

      scull_release函数实现关闭设备，`return 0`即可。

   -scull_read   

    ```C
      ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
      {
          struct scull_dev *dev = filp->private_data;
          ssize_t retval = 0;
 
          if (down_interruptible(&dev->sem))
          {
              return -ERESTARTSYS;
          }
          if (*f_pos >= dev->size)
          {
              goto out;
          }
          if (*f_pos + count > dev->size)
          {
              count = dev->size - *f_pos;
          }
 
          if (!dev->data)
          {
              goto out;
          }
 
          if (raw_copy_to_user(buf, dev->data + *f_pos, count))
          {
              retval = -EFAULT;
              goto out;
          }
 
          *f_pos += count;
          retval = count;
       
          out:
              up(&dev->sem);
              return retval;
      }
      ```
 

- scull_write

```C
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;
 
    if (down_interruptible(&dev->sem))   
    {
        return -ERESTARTSYS;
    }
 
    if (!dev->data)   
    {
        dev->data = kmalloc(SCULL_BUFFER_SIZE, GFP_KERNEL);
        if (!dev->data)
        {
            goto out;
        }
        memset(dev->data, 0, SCULL_BUFFER_SIZE);
     }
 
    if (count > SCULL_BUFFER_SIZE - dev->size)
    {
        count = SCULL_BUFFER_SIZE - dev->size;
    }
 
    if (raw_copy_from_user(dev->data + dev->size, buf, count))
    {
        retval = -EFAULT;
        goto out;
    }
     
    dev->size += count;
    retval = count;
 
    out:
        up(&dev->sem);
        return retval;
}
```

- scull_ioctl

```
long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct scull_dev *dev = filp->private_data;
    int err = 0, retval = 0;

    if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > SCULL_IOC_MAXNR)
        return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    switch (cmd) {
        case SCULL_IOC_CLEAR:
            dev->data_i = 0;
            printk(KERN_EMERG"SCULL_IOC_CLEAR data: 0\n");
            break;
        case SCULL_IOC_GET:
            retval = __put_user(dev->data_i, (int __user *)arg);
            printk(KERN_EMERG"SCULL_IOC_GET data: %d\n", dev->data_i);
            break;
        case SCULL_IOC_QUERY:
            printk(KERN_EMERG"SCULL_IOC_QUERY data: %d\n", dev->data_i);
            retval = dev->data_i;
            break;
        case SCULL_IOC_SET:
            retval = __get_user(dev->data_i, (int __user *)arg);
            printk(KERN_EMERG"SCULL_IOC_SET data: %d\n", dev->data_i);
            break;
        case SCULL_IOC_TELL:
            dev->data_i = arg;
            printk(KERN_EMERG"SCULL_IOC_TELL data: %d\n", arg);
            break;
        default:
            retval = -EINVAL;
            break;
    }

    return retval;
}
```

```c
loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;
 
    switch(whence)
    {
        case 0:
            newpos = off;
            break;
        case 1:
            newpos = filp->f_pos + off;
            break;
        case 2:
            newpos = dev->size + off;
            break;
        default:
            return -EINVAL;
    }
    if (newpos < 0)
    {
        return -EINVAL;
    }
    filp->f_pos = newpos;
    return newpos;
}
 
struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read = scull_read,
    .write = scull_write,
    .open = scull_open,
    .release = scull_release,
    .unlocked_ioctl = scull_ioctl,   
};
 
void  scull_cleanup_module(void)
{
    dev_t devno = MKDEV(scull_major, scull_minor);
 
    if (scull_device)
    {
        scull_trim(scull_device);
        cdev_del(&scull_device->cdev);
        kfree(scull_device);   
    }
    unregister_chrdev_region(devno, 1);
}
 
static void scull_setup_cdev(struct scull_dev *dev)
{
    int err, devno = MKDEV(scull_major, scull_minor);
 
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);
 
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding scull", err);
    }
}
```

- scull设备安装
本实验共创建两个scull设备，分别为scull0，scull1。采用在内核源码树内编译的方式。

将`scull0.c scull0.h scull1.c scull1.h`复制到内核源码树`/linux/drivers/char/`目录下，修改该目录下的kconfig，加入两个设备

![修改kconfig](./picture/修改kconfig.png)

在kconfig选项中加入`default m`可以默认将两个scull设备作为模块编译，而不用在menuconfig中进行选择

再在该目录的makefile中加入编译选项

![修改makefile](./picture/修改makefile.png)

在linux源码根目录打开终端，使用`make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules`进行编译，即可在`/linux/d
r/`目录下得到`scull0.ko scull1.ko`两个驱动文件。

![编译scull](./picture/编译scull.png)

将文件传输到树莓派端

![传输到树莓派](./picture/传输到树莓派.png)

在树莓派端使用`sudo insmod`命令加载驱动，`lsmod`可观察到两个设备加载成功。

![加载驱动](./picture/加载驱动.png)

输入`cat /proc/devices`获得驱动的主设备号，并根据设备号使用`sudo mknod``创建驱动文件

![设备号](./picture/设备号.png)

![挂载设备](./picture/挂载设备.png)

scull设备在树莓派上加载成功。

- scull设备测试

编写`test.c`文件对scull设备进行测试，主要测试其read、write、ioctl功能，test.c源码如下

```c
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
```

编译后运行./test，成功输出结果，左侧终端使用`sudo cat /proc/kmsg`用于观察printk输出的调试信息，右侧终端观察正常调试信息，可见read、write、ioctl功能正常，设备测试成功。

![驱动测试](./picture/驱动测试.png)

- 总结


### **四、实验总结**   

    
