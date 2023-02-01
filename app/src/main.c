#include <zephyr/kernel.h>

#if DEBUG == 1
#include <stdio.h>
#include "hid_msg.c"
#endif

#include "main.h"
#include "app_version.h"

#include "mpu9250.c"

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
    k_msleep(SLEEP_TIME_MS);

    mpu9250_init();

    int i = 0;
    while (1) {
        if (gpio_pin_toggle_dt(&led) < 0) {
            return;
        }
        if (mpu9250_process(mpu9250) != 0) {
//            return;
        }

        i++;
    #if DEBUG == 1
//        char buffer[12];
//        sprintf(buffer, "tick: %d\n", i);
//        write_message(buffer);
    #endif

        k_msleep(SLEEP_TIME_MS);
    }
}
