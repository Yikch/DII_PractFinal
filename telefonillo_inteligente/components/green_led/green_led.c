#include "green_led.h"
#include "esp_log.h"

static const char *TAG = "GREEN_LED";

esp_err_t green_led_init(void)
{
    // Configuración de la estructura del GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,               // Sin interrupción
        .mode = GPIO_MODE_OUTPUT_OD,                  // GPIO3 LED: open-drain, lógica normal según HW
        .pin_bit_mask = (1ULL << GREEN_LED_GPIO_NUM), // Máscara de bits para el GPIO3
        .pull_down_en = GPIO_PULLDOWN_DISABLE,        // Sin pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE,            // Sin pull-up
    };

    // Aplicar la configuración
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Fallo al configurar GPIO: %s", esp_err_to_name(err));
        return err;
    }

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "LED inicializado en GPIO%d", GREEN_LED_GPIO_NUM);
    }

    return err;
}

esp_err_t green_led_on(void)
{
    esp_err_t err = gpio_set_level(GREEN_LED_GPIO_NUM, 1);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "LED ENCENDIDO");
    }
    else
    {
        ESP_LOGE(TAG, "Fallo al encender el LED: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t green_led_off(void)
{
    esp_err_t err = gpio_set_level(GREEN_LED_GPIO_NUM, 0);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "LED APAGADO");
    }
    else
    {
        ESP_LOGE(TAG, "Fallo al apagar el LED: %s", esp_err_to_name(err));
    }
    return err;
}