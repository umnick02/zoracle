#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>

#include "main.h"
#include "app_version.h"
#include "mpu9250.c"
#include "sbus.h"
/* SBUS object, reading SBUS */
bfs::SbusRx sbus_rx(&Serial2);
/* SBUS object, writing SBUS */
bfs::SbusTx sbus_tx(&Serial2);
/* SBUS data */
bfs::SbusData data;
void setup() {
    /* Begin the SBUS communication */
    sbus_rx.Begin();
    sbus_tx.Begin();
    while (1) {
        if (sbus_rx.Read()) {
            /* Grab the received data */
            data = sbus_rx.data();
            /* Display the received data */
            for (int8_t i = 0; i < data.NUM_CH; i++) {
                LOG_INF(data.ch[i]);
                LOG_INF("\t");
            }
            /* Display lost frames and failsafe data */
            LOG_INF(data.lost_frame);
            LOG_INF("\t");
            LOG_INF(data.failsafe);
            /* Set the SBUS TX data to the received data */
            sbus_tx.data(data);
            /* Write the data to the servos */
            sbus_tx.Write();
        }
    }
}

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
    setup();

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
