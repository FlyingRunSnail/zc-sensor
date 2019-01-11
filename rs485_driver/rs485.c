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

#define RCV_BUF_LEN (16)

static pthread_mutex_t rs485_read_lock;
static pthread_mutex_t rs485_write_lock;

static int debug = 0;

#define RS485_LOG(FMT, ARG...) \
    do {\
            if (debug) \
            {\
                msg_log("[RS485]"FMT"\n", ##ARG); \
            }\
        }while(0)

void rs485_dump(const char *buf, unsigned int len)
{
    unsigned int i = 0;

    for (; i < len; i++)
    {
        RS485_LOG("%02x ", buf[i]);
    }
    RS485_LOG("");
}

int rs485_debug(int enable)
{
    debug = enable;
    return RS485_OK;
}

int rs485_init(const char *dev)
{
    int fd = open_serial(dev, 9600, 8, 'N', 1); 
    if (fd < 0)
    {
        RS485_LOG("open serial %s failed.\n", dev);
        return RS485_ERR;
    }

    pthread_mutex_init(&rs485_read_lock, NULL);
    pthread_mutex_init(&rs485_write_lock, NULL);

    return RS485_OK;
}

int rs485_send_command(const char *command)
{
    int ret;
    int len;
    int fd = get_fd();
    if (fd < 0) 
    {
        RS485_LOG("get invalid fd.\n");
        return RS485_ERR;
    }

    if (!command)
    {
        RS485_LOG("invalid input param frame.\n");
        return RS485_ERR;
    }

    len = strlen(command);

    pthread_mutex_lock(&rs485_write_lock);

    ret = write(fd, command, len);
    tcflush(fd, TCOFLUSH); 
    if (ret < 0)
    {
        pthread_mutex_unlock(&rs485_write_lock);
        RS485_LOG("send command failed.\n");
        tcflush(fd, TCOFLUSH); 
        return RS485_ERR;
    }

    pthread_mutex_unlock(&rs485_write_lock);

    return ret;
}

int rs485_rcv_response(char *response, unsigned int len, unsigned int timeout)
{
    int length = 0;
    int size = 0;
    int retval;
    int fd = -1;
    char buf[RCV_BUF_LEN] = {0};
    fd_set fs_read;
    struct timeval tv;

    fd = get_fd();
    if (fd < 0)
    {
        RS485_LOG("invalid fd.\n");
        return RS485_ERR;
    }

    if (!response)
    {
        RS485_LOG("invalid input param frame.\n");
        return RS485_ERR;
    }

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    tv.tv_sec  = timeout;
    tv.tv_usec = 0;
    retval = select(fd + 1, &fs_read, NULL, NULL, &tv);
    if (retval == -1)
    {
        perror("select()");
        return RS485_ERR;
    }
    else if (retval)
    {
        RS485_LOG("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
    }
    else
    {
        RS485_LOG("No data within five seconds.\n");
        return RS485_ERR;
    }

    if (FD_ISSET(fd, &fs_read))
    {
        pthread_mutex_lock(&rs485_read_lock);

        memset(buf, 0, RCV_BUF_LEN);

        do
        {
            size = read(fd, buf + length, RCV_BUF_LEN - length);
            if (size == 0)
            {
               usleep(4000); 
               size = read(fd, buf + length, RCV_BUF_LEN - length);
            }

            length += size;

        }while(size > 0);

        if (length < len)
        {
            pthread_mutex_unlock(&rs485_read_lock);
            RS485_LOG("length: %d and len: %d \n", length, len);
            rs485_dump(buf, length);
            return RS485_ERR;
        }
        
        rs485_dump(buf, length);
        strncpy(response, buf, len);
        pthread_mutex_unlock(&rs485_read_lock);

        return RS485_OK;
    }

    RS485_LOG("poll failed.\n");
    return RS485_ERR;
}

int rs485_send_and_wait_for(const char *command, const char *wait, unsigned int timeout)
{
    int ret;
    char buf[RCV_BUF_LEN] = {0};

    ret = rs485_send_command(command);
    if (ret < 0)
    {
        RS485_LOG("send command %s failed.\n", command);
        return RS485_ERR;
    }

    usleep(200*1000);
    ret = rs485_rcv_response(buf, strlen(wait), timeout);
    if (ret < 0)
    {
        RS485_LOG("send command %s and wait %s failed\n", command, wait);
        return RS485_ERR;
    }

    if (!strstr(buf, wait))
    {
        RS485_LOG("wait string %s not find in response %s\n", wait, buf);
        return RS485_ERR;
    }

    return RS485_OK;
}

void rs485_exit(void)
{
    close_serial();

    pthread_mutex_destroy(&rs485_read_lock);
    pthread_mutex_destroy(&rs485_write_lock);

    RS485_LOG("rs485 exit.");

    return ;
}

