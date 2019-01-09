
CC:=$(CROSS_COMPILE)gcc
CFLAGS:=-g -Werror -Wall -pthread
LDFLAGS:= -lm -pthread
TARGET:=zc-sensor
RM:=rm
INCLUDES=-Isensor_driver/ 
INCLUDES+=-Irs485_driver/
INCLUDES+=-Ilib/

OBJS=main.o sensor_driver/sensor.o rs485_driver/rs485.o rs485_driver/serial.o lib/msg_log.o

all:$(OBJS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS)

.c.o:
	$(CC) -c -o $*.o $< $(INCLUDES) $(CFLAGS)

.PHONY:all clean

clean:
	${RM} -f $(OBJS)
	${RM} -f $(TARGET)
