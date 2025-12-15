#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "esp_err.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define BUTTON_UP_GPIO_NUM GPIO_NUM_1
#define BUTTON_DOWN_INTR_FLAG GPIO_INTR_NEGEDGE
#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_0

#define UP_BUTTON_THRESHOLD_MV_MAX 400
#define UP_BUTTON_THRESHOLD_MV_MIN 345

#define ADC_READ_PERIOD_MS 500

#define ADC_HANDLER_BUSY_BIT BIT0

ESP_EVENT_DECLARE_BASE(ADC_BUTTON_EVENTS);

enum {
    ADC_BUTTON_SINGLE_PRESS_UP,
};

esp_err_t button_handler_init(EventGroupHandle_t main_adc_event_group);

#endif