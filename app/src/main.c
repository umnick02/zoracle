#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>

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
    hid_init();
    log_backend_enable(&log_backend_usb_hid, NULL, LOG_LEVEL_DBG);

    k_msleep(SLEEP_TIME_MS);
    mpu_init();

    while (1) {
        if (gpio_pin_toggle_dt(&led) < 0) {
            return;
        }
        if (mpu_process(mpu9250) != 0) {
//            return;
        }
        k_msleep(SLEEP_TIME_MS);
    }
}
