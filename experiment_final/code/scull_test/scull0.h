
#ifndef _SCULL_H
#define _SCULL_H
 
#define SCULL_MAJOR 0
#define SCULL_BUFFER_SIZE PAGE_SIZE
 
struct scull_dev {
    char *data;
    int data_i;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
};


#define SCULL_IOC_MAGIC '$'


#define SCULL_IOC_CLEAR _IO(SCULL_IOC_MAGIC, 0)

#define SCULL_IOC_GET   _IOR(SCULL_IOC_MAGIC, 1, int)

#define SCULL_IOC_QUERY _IO(SCULL_IOC_MAGIC, 2)

#define SCULL_IOC_SET   _IOW(SCULL_IOC_MAGIC, 3, int)

#define SCULL_IOC_TELL  _IO(SCULL_IOC_MAGIC, 4)
 
#endif

