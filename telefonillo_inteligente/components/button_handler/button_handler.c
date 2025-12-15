#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button_handler.h"

static const char *TAG = "BUTTON_HANDLER";
static adc_oneshot_unit_handle_t adc1_handle;
bool pressed = false;
static EventGroupHandle_t adc_event_group = NULL;

ESP_EVENT_DEFINE_BASE(ADC_BUTTON_EVENTS);

static void adc_button_handler(void *arg)
{
    int raw = 0;

    while (1)
    {
        esp_err_t err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
        EventBits_t bits = xEventGroupGetBits(adc_event_group);

        if (!(bits & ADC_HANDLER_BUSY_BIT))
        {
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Fallo en lectura ADC: %s", esp_err_to_name(err));
                vTaskDelay(pdMS_TO_TICKS(1000)); // Espera larga en caso de error
                continue;
            }

            int mv = raw * 3300 / 4095;

            //ESP_LOGI(TAG, "Vm value: %i", mv);

            bool is_button_down = (mv >= UP_BUTTON_THRESHOLD_MV_MIN) && (mv <= UP_BUTTON_THRESHOLD_MV_MAX);

            if (is_button_down && !pressed)
            {
                ESP_LOGI(TAG, "¡Pulsación UP+ detectada!");
                esp_event_post(ADC_BUTTON_EVENTS, ADC_BUTTON_SINGLE_PRESS_UP, NULL, 0, portMAX_DELAY);
                pressed = true;
            }
            else
            {
                if (pressed)
                {
                    pressed = false;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ADC_READ_PERIOD_MS));
    }
}

esp_err_t button_handler_init(EventGroupHandle_t main_adc_event_group)
{

    adc_event_group = main_adc_event_group;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,         // Atenuación de 12dB (rango completo)
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Ancho de bits por defecto
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));

    if (xTaskCreate(adc_button_handler, "adc_monitor", 4096, NULL, 5, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "No se pudo crear la tarea para la pulsación del botón.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "ADC Button Handler inicializado para pulsación simple en GPIO1 (ADC1_CH0)");
    return ESP_OK;
}