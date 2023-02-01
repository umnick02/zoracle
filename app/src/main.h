#ifndef ZORACLE_MAIN_H
#define ZORACLE_MAIN_H

#define DEBUG 1
#define SLEEP_TIME_MS   1000

#include <stdio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

/* The devicetree node identifier for the "led0" alias. */
#include <zephyr/drivers/gpio.h>
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const char *now_str(void) {
    static char buf[16]; /* ...HH:MM:SS.MMM */
    uint32_t now = k_uptime_get_32();
    unsigned int ms = now % 1000;
    unsigned int s;
    unsigned int min;
    unsigned int h;

    now /= 1000;
    s = now % 60U;
    now /= 60U;
    min = now % 60U;
    now /= 60U;
    h = now;

    snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u", h, min, s, ms);
    return buf;
}

#endif //ZORACLE_MAIN_H
