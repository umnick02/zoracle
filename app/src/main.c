/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hid_write.c"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#include "app_version.h"
#include <stdio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
char *hid_msg;

//static const char *now_str(void)
//{
//    static char buf[16]; /* ...HH:MM:SS.MMM */
//    uint32_t now = k_uptime_get_32();
//    unsigned int ms = now % MSEC_PER_SEC;
//    unsigned int s;
//    unsigned int min;
//    unsigned int h;
//
//    now /= MSEC_PER_SEC;
//    s = now % 60U;
//    now /= 60U;
//    min = now % 60U;
//    now /= 60U;
//    h = now;
//
//    snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
//             h, min, s, ms);
//    return buf;
//}

//
//static int process_mpu9250(const struct device *dev) {
//    struct sensor_value temperature;
//    struct sensor_value accel[3];
//    struct sensor_value gyro[3];
//    int rc = sensor_sample_fetch(dev);
//
//    if (rc == 0) {
//        rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
//    }
//    if (rc == 0) {
//        rc = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyro);
//    }
//    if (rc == 0) {
//        rc = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
//    }
//    if (rc == 0) {
//        sprintf(hid_msg, "[%s]:%g Cel\n"
//                         "  accel %f %f %f m/s/s\n"
//                         "  gyro  %f %f %f rad/s\n",
//                now_str(),
//                sensor_value_to_double(&temperature),
//                sensor_value_to_double(&accel[0]),
//                sensor_value_to_double(&accel[1]),
//                sensor_value_to_double(&accel[2]),
//                sensor_value_to_double(&gyro[0]),
//                sensor_value_to_double(&gyro[1]),
//                sensor_value_to_double(&gyro[2]));
//    } else {
//        sprintf(hid_msg, "sample fetch/get failed: %d\n", rc);
//    }
//    write_message(hid_msg);
//    return rc;
//}

//#ifdef CONFIG_MPU9250_TRIGGER
//static struct sensor_trigger trigger;
//
//static void handle_mpu9250_drdy(const struct device *dev, const struct sensor_trigger *trig)
//{
//	int rc = process_mpu9250(dev);
//
//	if (rc != 0) {
//		sprintf(hid_msg, "cancelling trigger due to failure: %d\n", rc);
//		(void)sensor_trigger_set(dev, trig, NULL);
//		return;
//	}
//}
//#endif /* CONFIG_MPU9250_TRIGGER */

void main(void)
{
    int ret;

    if (!device_is_ready(led.port)) {
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return;
    }

//    const struct device *const mpu9250 = DEVICE_DT_GET_ONE(invensense_mpu9250);
//
//    if (!device_is_ready(mpu9250)) {
//        sprintf(hid_msg, "Device %s is not ready\n", mpu9250->name);
//        write_message(hid_msg);
//        return;
//    }

//#ifdef CONFIG_MPU9250_TRIGGER
//    trigger = (struct sensor_trigger) {
//		.type = SENSOR_TRIG_DATA_READY,
//		.chan = SENSOR_CHAN_ALL,
//	};
//	if (sensor_trigger_set(mpu9250, &trigger, handle_mpu9250_drdy) < 0) {
//		sprintf(hid_msg, "Cannot configure trigger\n");
//		return;
//	}
//	sprintf(hid_msg, "Configured for triggered sampling.\n");
//#endif

    int i = 0;
    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return;
        }
//        int rc = process_mpu9250(mpu9250);
//
//        if (rc != 0) {
//            break;
//        }
        i++;
        sprintf(hid_msg, "msg: %d", i);
        write_message(hid_msg);
        k_msleep(SLEEP_TIME_MS);
    }
}
