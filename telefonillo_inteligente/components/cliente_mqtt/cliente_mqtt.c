#include "cliente_mqtt.h"
#include "esp_log.h"
#include <cJSON.h>

static const char *TAG = "MQTT";

nvs_handle_t *mqtt_nvs_hnd_ptr = NULL;

static esp_mqtt_client_handle_t mqtt_client = NULL;
bool mqtt_connected = false;

static char *global_broker_url = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    mqtt_client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        break;
    default:
        break;
    }
}

void Mqtt_client_start(nvs_handle_t *nvs_hnd)
{
    esp_err_t err = ESP_OK;
    size_t required_size = 0;

    mqtt_nvs_hnd_ptr = nvs_hnd;

    ESP_LOGI(TAG, "Iniciando cliente MQTT....");

    // Obtener URL del Broker desde la NVS si se indicó por provisionamiento SoftAP
    err = nvs_get_str(*mqtt_nvs_hnd_ptr, "MQTT_BROKER_URL", NULL, &required_size);
    if (err == ESP_OK)
    {
        global_broker_url = malloc(required_size);
        err = nvs_get_str(*mqtt_nvs_hnd_ptr, "MQTT_BROKER_URL", global_broker_url, &required_size);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "URL Broker almacenada en NVS = %s", global_broker_url);
        }
        else
        {
            ESP_LOGE(TAG, "Error obteniendo URL Broker MQTT desde NVS. %s \n"
                          "Usando URL por defecto configurada con MenuConfig %s",
                     esp_err_to_name(err), CONFIG_BROKER_URL);

            free(global_broker_url); // Liberar memoria del antiguo malloc

            required_size = strlen(CONFIG_BROKER_URL);
            global_broker_url = malloc(required_size + 1);
            if (global_broker_url != NULL)
            {
                strcpy(global_broker_url, CONFIG_BROKER_URL);
                global_broker_url[required_size] = '\0';
            }
        }
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG, "URL del broker no provisionada por SoftAP!\n"
                      "Provisione de nuevo con el parametro --customdata \"url_servidor_mqtt\" para usar una URL diferente\n"
                      "Usando URL por defecto %s",
                 CONFIG_BROKER_URL);

        required_size = strlen(CONFIG_BROKER_URL);
        global_broker_url = malloc(required_size + 1);
        if (global_broker_url != NULL)
        {
            strcpy(global_broker_url, CONFIG_BROKER_URL);
            global_broker_url[required_size] = '\0';
        }
    }
    else
    { // Otro error, notificar
        ESP_LOGE(TAG, "Error leyendo clave MQTT_BROKER_URL de la NVS: %s", esp_err_to_name(err));
    }

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = global_broker_url;
    mqtt_cfg.broker.address.port = 1883;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    ESP_LOGI(TAG, "MQTT client started");
}

void Mqtt_send_data()
{
    char msg[128];
    snprintf(msg, sizeof(msg), "CARA DETECTADA");

    esp_mqtt_client_publish(mqtt_client, CONFIG_ALARM_TOPIC, msg, 0, 1, 0);
    ESP_LOGI(TAG, "Alarma enviada por MQTT al broker %s:%d en TOPIC %s: %s", global_broker_url, 1883, CONFIG_ALARM_TOPIC, msg);
}

void Mqtt_client_free()
{
    esp_err_t errcode = ESP_OK;
    if ((errcode = esp_mqtt_client_stop(mqtt_client)) != ESP_OK)
        ESP_LOGE(TAG, "Error stoping mqtt client: %s", esp_err_to_name(errcode));

    esp_mqtt_client_destroy(mqtt_client);
}