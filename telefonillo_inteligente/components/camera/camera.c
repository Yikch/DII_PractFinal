#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "camera.h"

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 4, // 0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,      // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST,
};

static const char *TAG = "camera";

esp_err_t camera_init(void)
{
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);

    sensor_t *s = esp_camera_sensor_get();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    if (s->set_vflip(s, 1) != 0)
    {
        ESP_LOGE(TAG, "Failed to mirror the frame vertically.");
        return ESP_FAIL;
    }
    if (s->set_hmirror(s, 1) != 0)
    {
        ESP_LOGE(TAG, "Failed to mirror the frame horizontally.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Camera Initialize correctly");
    return ESP_OK;
}

camera_fb_t *get_capture(void)
{
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        ESP_LOGE(TAG, "Camera Capture Failed");
    }

    return fb;
}

void free_camera_buffer(camera_fb_t *fb)
{
    esp_camera_fb_return(fb);
}