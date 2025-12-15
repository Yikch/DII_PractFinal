#ifndef MQTT_H_
#define MQTT_H_

#include <nvs.h>
#include <nvs_flash.h>

// MQTT
#include <mqtt_client.h>

void Mqtt_client_start(nvs_handle_t *nvs_hnd);

void Mqtt_send_data();

void Mqtt_client_free();

#endif /* MQTT_H_ */