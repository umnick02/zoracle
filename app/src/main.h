#ifndef ZORACLE_MAIN_H
#define ZORACLE_MAIN_H

#define DEBUG 1
#define SLEEP_TIME_MS   1000

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

/* The devicetree node identifier for the "led0" alias. */
#include <zephyr/drivers/gpio.h>
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#endif //ZORACLE_MAIN_H
