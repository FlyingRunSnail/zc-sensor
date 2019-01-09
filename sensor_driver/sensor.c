#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "sensor.h"
#include "rs485.h"
#include "msg_log.h"

static int debug = 1;
#define SENSOR_LOG(FMT, ARG...) \
    do {\
            if (debug) \
            {\
                msg_log("[ZC-SENSOR]"FMT"\n", ##ARG); \
            }\
        }while(0)

static int sensor_init(void)
{
    /* 设置ASCII码模式 */
    rs485_send_and_wait_for("COMHOF", "F", 1);
    /* 单次输出模式 */
    rs485_send_and_wait_for("COMOFF", "F", 1);

    return RS485_OK;
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
    SENSOR_LOG("bsp sensor get current pos!");

    if (!ret_pos)
    {
        SENSOR_LOG("invalid outputt param!");
        return ZC_SENSOR_ERR;
    }

    rs485_send_command("SENDDD");
    rs485_rcv_response(buf);
    
    *ret_pos = strtof(&buf[1], NULL);
    
    return ZC_SENSOR_OK;
}

static int sensor_set_zero_pos(void)
{
    rs485_send_and_wait_for("&ZEROP", "SA", 1);
    return ZC_SENSOR_OK;
}

int bsp_sensor_set_zero_pos(void)
{
    SENSOR_LOG("bsp sensor set zero pos!");
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

