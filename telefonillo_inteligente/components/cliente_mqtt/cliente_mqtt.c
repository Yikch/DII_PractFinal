#include "cliente_mqtt.h"
#include "esp_log.h"
#include <cJSON.h>
#include "mbedtls/base64.h"

static const char *TAG = "MQTT";

// Plantilla base del topic
// El topic final se construye usando las macros CONFIG_ALARM_TOPIC/CONFIG_DEVICE_NAME
#define BASE_TOPIC "%s/%s"

nvs_handle_t *mqtt_nvs_hnd_ptr = NULL;

static esp_mqtt_client_handle_t mqtt_client = NULL;
bool mqtt_connected = false;

static char *global_broker_url = NULL;
static char *global_topic = NULL;

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
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
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
                      "Usando URL por defecto %s:%d",
                 CONFIG_BROKER_URL,
                 CONFIG_BROKER_PORT);

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
    mqtt_cfg.broker.address.port = CONFIG_BROKER_PORT;
    mqtt_cfg.buffer.out_size = 15360;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // *** Crear el topic para enviar los datos del nodo ***
    required_size = snprintf(NULL, 0, BASE_TOPIC, CONFIG_ALARM_TOPIC, CONFIG_DEVICE_NAME);
    if (required_size < 0)
    {
        ESP_LOGE(TAG, "Error al crear el topic de publicacion");
    }
    else
    {
        global_topic = malloc(required_size + 1);
        if (global_topic != NULL)
        {
            snprintf(global_topic, required_size + 1, BASE_TOPIC, CONFIG_ALARM_TOPIC, CONFIG_DEVICE_NAME);
            ESP_LOGI(TAG, "Topic de publicaciones del telefonillo: %s", global_topic);
        }
    }

    ESP_LOGI(TAG, "MQTT client started");
}

void Mqtt_send_base64_image(const uint8_t *image_data, size_t image_size)
{
    char *base64_encoded_str = NULL;
    size_t len_requerida = 0; // Usado para calcular el tamaño de malloc
    size_t len_escrita = 0;   // Usado para la longitud real de la cadena (el payload)
    int ret;                  // Código de retorno de la función mbedtls

    // --- 1. Calcular la longitud de salida requerida ---
    // Llamamos a la función con el buffer de salida NULL o tamaño 0.
    // El segundo parámetro (len_requerida) devolverá el tamaño necesario.
    ret = mbedtls_base64_encode(
        NULL,           // Output buffer: NULL
        0,              // Output buffer size: 0
        &len_requerida, // OUT: Aquí se guarda el tamaño que necesitamos
        image_data,     // Input data
        image_size      // Input data size
    );

    if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL)
    {
        // En este paso, el error esperado es BUFFER_TOO_SMALL, lo cual
        // nos dice que se calculó la longitud correctamente. Si es otro error, fallamos.
        ESP_LOGE(TAG, "Error 1 al calcular longitud Base64: 0x%x", ret);
        return;
    }

    // Si la imagen es vacía, salimos
    if (len_requerida == 0)
    {
        ESP_LOGE(TAG, "La imagen está vacía.");
        return;
    }

    // --- 2. Asignar memoria dinámica para la cadena Base64 ---
    // La longitud requerida de mbedtls_base64_encode YA INCLUYE el terminador '\0'.
    // Por lo tanto, no necesitamos añadir un '+ 1'.
    base64_encoded_str = (char *)malloc(len_requerida);

    if (base64_encoded_str == NULL)
    {
        ESP_LOGE(TAG, "Error: No se pudo asignar memoria para Base64.");
        return;
    }

    // --- 3. Codificar la imagen a Base64 ---
    // Llama a la función de nuevo, esta vez con el buffer y el tamaño correctos.
    ret = mbedtls_base64_encode(
        (unsigned char *)base64_encoded_str, // Output buffer
        len_requerida,                       // Output buffer size
        &len_escrita,                        // OUT: Longitud REAL escrita (sin el '\0')
        image_data,                          // Input data
        image_size                           // Input data size
    );

    if (ret != 0)
    {
        ESP_LOGE(TAG, "Error 2 al codificar Base64: 0x%x", ret);
        free(base64_encoded_str);
        return;
    }

    // --- 4. Publicar la cadena codificada ---
    int msg_id = esp_mqtt_client_publish(mqtt_client, global_topic, base64_encoded_str, len_escrita, 1, 0);
    ESP_LOGI(TAG, "Imagen Base64 enviada por MQTT al broker %s:%d en TOPIC %s", global_broker_url, CONFIG_BROKER_PORT, global_topic);
    ESP_LOGI(TAG, "Longitud: %zu bytes. ID: %d", len_escrita, msg_id);

    ESP_LOGI(TAG, "Imagen Base64 publicada. Longitud: %zu bytes. ID: %d", len_escrita, msg_id);

    // --- 5. Liberar memoria asignada ---
    free(base64_encoded_str);
}

void Mqtt_client_free()
{
    esp_err_t errcode = ESP_OK;
    if ((errcode = esp_mqtt_client_stop(mqtt_client)) != ESP_OK)
        ESP_LOGE(TAG, "Error stoping mqtt client: %s", esp_err_to_name(errcode));

    esp_mqtt_client_destroy(mqtt_client);

    free(global_topic);
    free(global_broker_url);
}