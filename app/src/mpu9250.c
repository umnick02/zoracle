#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
//#include "mpu9250/mpu9250.c"
#include <stdio.h>

const struct device *mpu9250;

struct sensor_value temperature;
struct sensor_value accel[3];
struct sensor_value gyro[3];
struct sensor_value magn[3];

static int mpu_process(const struct device *dev);
static void mpu_init();

static void mpu_init() {
    mpu9250 = DEVICE_DT_GET_ONE(invensense_mpu9250);
//    mpu9250_init(mpu9250);
    if (!device_is_ready(mpu9250)) {
        LOG_ERR("Device %s is not ready\n", mpu9250->name);
    }
}

static int mpu_process(const struct device *dev) {
    int rc = sensor_sample_fetch(dev);

    if (rc == 0) {
        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
    }
    if (rc == 0) {
        rc = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyro);
    }
    if (rc == 0) {
        rc = sensor_channel_get(dev, SENSOR_CHAN_MAGN_XYZ, magn);
    }
    if (rc == 0) {
        sensor_channel_get(dev, SENSOR_CHAN_DIE_TEMP, &temperature);
    }
    if (rc == 0) {
        LOG_INF("%g Cel\naccel %f %f %f m/s^2\ngyro  %f %f %f rad/s\nmagn  %f %f %f G\n",
                sensor_value_to_double(&temperature),
                sensor_value_to_double(&accel[0]),
                sensor_value_to_double(&accel[1]),
                sensor_value_to_double(&accel[2]),
                sensor_value_to_double(&gyro[0]),
                sensor_value_to_double(&gyro[1]),
                sensor_value_to_double(&gyro[2]),
                sensor_value_to_double(&magn[0]),
                sensor_value_to_double(&magn[1]),
                sensor_value_to_double(&magn[2]));
    } else {
        LOG_ERR("sample fetch/get failed: %d\n", rc);
    }
    return rc;
}
