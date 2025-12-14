#ifndef GREEN_LED_H
#define GREEN_LED_H

#include <driver/gpio.h>
#include <esp_err.h>

#define GREEN_LED_GPIO_NUM  GPIO_NUM_3

esp_err_t green_led_init(void);

esp_err_t green_led_on(void);

esp_err_t green_led_off(void);

#endif // LED_CONTROL_H