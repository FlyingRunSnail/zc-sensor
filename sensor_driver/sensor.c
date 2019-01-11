#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "sensor.h"
#include "rs485.h"
#include "msg_log.h"

#define MODULE_NAME "[ZC-SENSOR]"

static int debug = 1;
#define SENSOR_LOG(FMT, ARG...) \
    do {\
            if (debug) \
            {\
                msg_log(MODULE_NAME FMT"\n", ##ARG); \
            }\
        }while(0)

static int sensor_init(void)
{
    /* 停止发送，停止接收，等待接收命令 */
    rs485_send_command("$");
    rs485_send_command("$");

    /* 设置ASCII码模式 */
    rs485_send_command("COMHOF");
    rs485_send_command("$");
    rs485_send_command("$");

    /* 单次输出模式 */
    rs485_send_command("COMOFF");
    rs485_send_command("$");
    rs485_send_command("$");

    return RS485_OK;
}

int bsp_sensor_debug(int enable)
{
    debug = enable;
    return ZC_SENSOR_OK;
}

int bsp_sensor_init(void)
{
    rs485_init("/dev/ttyUSB0");
    SENSOR_LOG("bsp sensor init ok!");

    return ZC_SENSOR_OK;
}

int bsp_sensor_get_current_pos(float *ret_pos)
{
    char buf[16] = {0};
    int ret;

    SENSOR_LOG("bsp sensor get current pos!");

    if (!ret_pos)
    {
        SENSOR_LOG("invalid output param!");
        return ZC_SENSOR_ERR;
    }

    rs485_send_command("SENDDD");

    /* 发送命令后必须等待一段时间，否则接收返回信息不完整  */
    usleep(200*1000);
    ret = rs485_rcv_response(buf, 9, 5);
    if (ret < 0)
    {
        SENSOR_LOG("rcv current pos failed.");
        return ZC_SENSOR_ERR;
    }

    if((buf[0] != 'X') || (buf[1] != '=') || (buf[7] != 0x0d) || (buf[8] != 0x0a))
    {
        SENSOR_LOG("check rcved response failed.");
        rs485_dump(buf, 9);
        return ZC_SENSOR_ERR;
    }

    /* 将\r\n转换为0  */
    buf[7] = 0;
    buf[8] = 0;
    
    *ret_pos = strtof(&buf[2], NULL);

    SENSOR_LOG("current pos: %f", *ret_pos);
    
    return ZC_SENSOR_OK;
}

static int sensor_set_zero_pos(void)
{
    char buf[8] = {0};

    //rs485_send_and_wait_for("&ZEROP", "SA", 1);
    rs485_send_command("&ZEROP");
    usleep(100*1000);
    rs485_rcv_response(buf, 2, 5);

    //if (!strstr(buf, "SA"))
    if (!strstr(buf, "X"))
    {
        SENSOR_LOG("no response or do not receive response.");
        rs485_dump(buf, 8);
        return ZC_SENSOR_ERR;
    }

    return ZC_SENSOR_OK;
}

int bsp_sensor_set_zero_pos(void)
{
    SENSOR_LOG("bsp sensor set zero pos.");
    sensor_init();
    sensor_set_zero_pos();

    return ZC_SENSOR_OK;
}

void bsp_sensor_exit(void)
{
    rs485_exit();
    SENSOR_LOG("bsp sensor exit!");
    return ;
}

