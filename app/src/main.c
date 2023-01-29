#include "main.h"
#include "app_version.h"

#include <zephyr/kernel.h>

#include "mpu9250.c"

#if DEBUG == 1
#include <stdio.h>
#include "hid_msg.c"
#endif

void main(void) {
    LOG_INF("Starting application");

    if (!device_is_ready(led.port)) {
        return;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        return;
    }

#if DEBUG == 1
    hid_init();
#endif

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
        if (gpio_pin_toggle_dt(&led) < 0) {
            return;
        }
//        if (process_mpu9250(mpu9250) != 0) {
//            break;
//        }
        i++;
    #if DEBUG == 1
        char buffer[12];
        sprintf(buffer, "tick: %d\n", i);
        write_message(buffer);
    #endif
        k_msleep(SLEEP_TIME_MS);
    }
}
