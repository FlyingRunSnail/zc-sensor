#ifndef __RS485_HEADER__
#define __RS485_HEADER__

#define RS485_OK  (0)
#define RS485_ERR (-1)

int rs485_init(const char *dev);
void rs485_exit(void);

int rs485_send_command(const char *command);
int rs485_rcv_response(char *response, unsigned int len, unsigned int timeout);
int rs485_send_and_wait_for(const char *command, const char *wait, unsigned int timeout);

/* enable=1 打开debug使能， enable=0 关闭enable使能 */
int rs485_debug(int enable);
void rs485_dump(const char *buf, unsigned int len);

#endif
