#ifndef WIFI_H_
#define WIFI_H_

#include <esp_wifi.h>
#include <nvs.h>
#include <nvs_flash.h>

ESP_EVENT_DECLARE_BASE(MY_WIFI_EVENT);

#define MY_WIFI_CONNECTED_WITH_IP_EVENT 0
#define MY_WIFI_DISCONNECTED_EVENT 1

esp_err_t wifi_init(nvs_handle_t *nvs_hnd);
void wifi_power_save(wifi_ps_type_t modo);

#endif