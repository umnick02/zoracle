#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include <stdio.h>

#if DEBUG == 1
#include "hid_msg.c"
#endif

const struct device *mpu9250;

struct sensor_value temperature;
struct sensor_value accel[3];
struct sensor_value gyro[3];
struct sensor_value magn[3];

static int mpu9250_process(const struct device *dev);
static void mpu9250_init();

#ifdef CONFIG_MPU9250_TRIGGER
static struct sensor_trigger trigger;

static void handle_mpu9250_drdy(const struct device *dev, const struct sensor_trigger *trig) {
	int rc = mpu9250_process(dev);

	if (rc != 0) {
        char buffer[127];
		sprintf(buffer, "cancelling trigger due to failure: %d\n", rc);
        LOG_ERR("%s", buffer);
    #if DEBUG == 1
        write_message(buffer);
    #endif
		(void)sensor_trigger_set(dev, trig, NULL);
		return;
	}
}
#endif /* CONFIG_MPU9250_TRIGGER */

static void mpu9250_init() {
    mpu9250 = DEVICE_DT_GET_ONE(invensense_mpu9250);

    if (!device_is_ready(mpu9250)) {
        char buffer[32];
        sprintf(buffer, "Device %s is not ready\n", mpu9250->name);
        LOG_ERR("%s", buffer);
    #if DEBUG == 1
        write_message(buffer);
    #endif
        return;
    }

#ifdef CONFIG_MPU9250_TRIGGER
    char buffer[32];
    trigger = (struct sensor_trigger) {
		.type = SENSOR_TRIG_DATA_READY,
		.chan = SENSOR_CHAN_ALL,
	};
	if (sensor_trigger_set(mpu9250, &trigger, handle_mpu9250_drdy) < 0) {
		sprintf(buffer, "Cannot configure trigger\n");
        LOG_ERR("%s", buffer);
    #if DEBUG == 1
        write_message(buffer);
    #endif
		return;
	}
	sprintf(buffer, "Configured for triggered sampling.\n");
    LOG_INF("%s", buffer);
#if DEBUG == 1
    write_message(buffer);
#endif
#endif
}

static int mpu9250_process(const struct device *dev) {
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
    char buffer[127];
    if (rc == 0) {
        sprintf(buffer, "[%s]:\n"
                         "  %g Cel\n"
                         "  accel %f %f %f m/s/s\n"
                         "  gyro  %f %f %f rad/s\n"
                         "  magn  %f %f %f nT\n",
                now_str(),
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
        LOG_INF("%s", buffer);
    } else {
        sprintf(buffer, "sample fetch/get failed: %d\n", rc);
        LOG_ERR("%s", buffer);
    }
#if DEBUG == 1
    write_message(buffer);
#endif
    return rc;
}
