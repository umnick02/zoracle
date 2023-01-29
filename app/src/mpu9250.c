//#include <zephyr/kernel.h>
//#include <zephyr/device.h>
//#include <zephyr/drivers/sensor.h>
//
//#if DEBUG == 1
//#include <stdio.h>
//#include "hid_msg.c"
//#endif
//
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
//    char hid_msg[8];
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
//
//
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