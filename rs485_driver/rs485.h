#ifndef __RS485_HEADER__
#define __RS485_HEADER__

#define RS485_OK  (0)
#define RS485_ERR (-1)

int rs485_init(const char *dev);
void rs485_exit(void);

int rs485_send_command(const char *command);
int rs485_rcv_response(char *response);
int rs485_send_and_wait_for(const char *command, const char *wait, unsigned int timeout);

#endif
