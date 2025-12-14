#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_check.h"
#include "esp_err.h"
#include <nvs.h>
#include "camera.h"
#include "face_recognition.h"
#include "green_led.h"
#include "wifi.h"
#include <string.h>

#define PROVISIONING_SOFTAP

static char *TAG = "MAIN";

// wifi_credentials_t wifi_credentials;
nvs_handle_t my_nvs_hnd;

esp_err_t initNVS()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        err = nvs_flash_erase();
        if (err != ESP_OK)
            ESP_LOGE(TAG, "Error in nvs_flash_erase: %s", esp_err_to_name(err));
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
        return err; // Error, retornar

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    err = nvs_open("storage", NVS_READWRITE, &my_nvs_hnd);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Done");
    }

    return err;
}

void app_main(void)
{
    // Inicializar NVS
    if (initNVS() != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar NVS.");
        return;
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Inicializar wifi.
    // El handler de la NVS es para que se guarde la URL del Broker de MQTT y no se pierda tras reiniciar la placa
    if (wifi_init(&my_nvs_hnd) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al inicializar modulo WiFi.");
    }
    else
    {
        ESP_LOGI(TAG, "Conectado a la red wifi.");
    }

    ESP_ERROR_CHECK(camera_init());
    ESP_ERROR_CHECK(face_recognition_init());
    ESP_ERROR_CHECK(green_led_init());

    green_led_off();
    while (1)
    {
        camera_fb_t *fb = get_capture();
        //ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", fb->len);
        float score = recognize_face(fb->buf, fb->len);

        if(score > 0.0f){ESP_LOGI(TAG, "score: %f", score);}

        free_camera_buffer(fb);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
