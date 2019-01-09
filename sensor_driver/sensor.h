#ifndef __ZC_SENSOR_DRIVER__
#define __ZC_SENSOR_DRIVER__

/* sensor模块接口函数返回值 */
#define ZC_SENSOR_OK  (0)
#define ZC_SENSOR_ERR (-1)

int bsp_sensor_init(void);
void bsp_sensor_exit(void);

int bsp_sensor_get_current_pos(float *ret_pos);
int bsp_sensor_set_zero_pos(void);

#endif
