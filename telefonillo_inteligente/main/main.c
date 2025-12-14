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
#include "camera.h"
#include "face_recognition.h"
#include "green_led.h"
#include <string.h>

#define PROVISIONING_SOFTAP

static char *TAG = "MAIN";

// wifi_credentials_t wifi_credentials;

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

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
