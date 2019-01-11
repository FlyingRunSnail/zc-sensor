#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sensor.h"
#include "rs485.h"

static void usage(void)
{
    printf("usage:\n \
            zc-sensor pos  #get current position\n \
            zc-sensor zero #set current position to zero\n \
            \n");
}

int main(int argc, char **argv)
{
    int ret;
    long debug = 0;
    int enable = 0;
    float ret_val = 0.0f;
    
    if (argc < 2 || argc > 3)
    {
        usage();
        return -1;
    }

    if (argc == 3)
    {
        debug = strtol(argv[argc-1], NULL, 10);

        if (debug > 0)
        {
            enable = 1;
        }

        bsp_sensor_debug(enable);
        rs485_debug(enable);
    }
    
    bsp_sensor_init();

    if (strcmp(argv[1], "zero") == 0)
    {
        ret = bsp_sensor_set_zero_pos();
        printf("power off the sensor and repower again\n");
    }
    else if (strcmp(argv[1], "pos") == 0)
    {
        ret = bsp_sensor_get_current_pos(&ret_val);
        if (ret != ZC_SENSOR_OK)
        {
            printf("get current pos failed.\n");
        }
        else
        {
            printf("current pos %f \n", ret_val);
        }
    }
    else
    {
        usage();
    }

    bsp_sensor_exit();

    return 0;
}

