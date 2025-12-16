#ifndef MQTT_H_
#define MQTT_H_

#include <nvs.h>
#include <nvs_flash.h>

// MQTT
#include <mqtt_client.h>

void Mqtt_client_start(nvs_handle_t *nvs_hnd);

void Mqtt_client_free();

void Mqtt_send_base64_image(const uint8_t *image_data, size_t image_size);

#endif /* MQTT_H_ */