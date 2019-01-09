#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "msg_log.h"
#include "rs485.h"
#include "serial.h"

#define RCV_BUF_LEN (8)
static unsigned char rs485_rcv_buf[RCV_BUF_LEN] = {0};

static pthread_mutex_t rs485_read_lock;
static pthread_mutex_t rs485_write_lock;

static int debug = 1;

#define RS485_LOG(FMT, ARG...) \
    do {\
            if (debug) \
            {\
                msg_log("[RS485]"FMT"\n", ##ARG); \
            }\
        }while(0)

int rs485_init(const char *dev)
{
    int fd = open_serial(dev, 9600, 8, 'N', 1); 
    if (fd < 0)
    {
        RS485_LOG("open serial %s failed.\n", dev);
        return -1;
    }

    pthread_mutex_init(&rs485_read_lock, NULL);
    pthread_mutex_init(&rs485_write_lock, NULL);

    return 0;
}

int rs485_send_command(const char *command)
{
    int ret;
    int len;
    int fd = get_fd();
    if (fd < 0) 
    {
        RS485_LOG("get invalid fd.\n");
        return -1;
    }

    if (!command)
    {
        RS485_LOG("invalid input param frame.\n");
        return -1;
    }

    len = strlen(command);
    pthread_mutex_lock(&rs485_write_lock);

    ret = write(fd, command, len);
    tcflush(fd, TCOFLUSH); 
    if (ret < 0)
    {
        pthread_mutex_unlock(&rs485_write_lock);
        RS485_LOG("send frame failed.\n");
        tcflush(fd, TCOFLUSH); 
        return -1;
    }

    pthread_mutex_unlock(&rs485_write_lock);

    return ret;
}

int rs485_rcv_response(char *response)
{
    int length = 0;
    int size = 0;
    int retval;
    int fd = -1;
    fd_set fs_read;
    struct timeval tv;

    fd = get_fd();
    if (fd < 0)
    {
        RS485_LOG("invalid fd.\n");
        return -1;
    }

    if (!response)
    {
        RS485_LOG("invalid input param frame.\n");
        return -1;
    }

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    retval = select(fd + 1, &fs_read, NULL, NULL, &tv);
    if (retval == -1)
    {
        perror("select()");
        return -1;
    }
    else if (retval)
    {
        RS485_LOG("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
    }
    else
    {
        RS485_LOG("No data within five seconds.\n");
        return -1;
    }

    if (FD_ISSET(fd, &fs_read))
    {
        pthread_mutex_lock(&rs485_read_lock);

        memset(rs485_rcv_buf, 0, RCV_BUF_LEN);

        do
        {
            size = read(fd, rs485_rcv_buf + length, RCV_BUF_LEN - length);
            if (size == 0)
            {
               usleep(4000);
               size = read(fd, rs485_rcv_buf + length, RCV_BUF_LEN - length);
            }

            length += size;

        }while(size > 0);

        if (length == 0)
        {
            pthread_mutex_unlock(&rs485_read_lock);
            RS485_LOG("length: %d \n", length);
            return -1;
        }
        
        pthread_mutex_unlock(&rs485_read_lock);

        return 0;
    }

    RS485_LOG("poll failed.\n");
    return -1;
}

int rs485_send_and_wait_for(const char *command, const char *wait, unsigned int timeout)
{
    return RS485_OK;
}

void rs485_exit(void)
{
    close_serial();

    pthread_mutex_destroy(&rs485_read_lock);
    pthread_mutex_destroy(&rs485_write_lock);

    return ;
}

