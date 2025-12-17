#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include <wifi_provisioning/manager.h>

#include <wifi_provisioning/scheme_softap.h>

#include "wifi.h"

ESP_EVENT_DEFINE_BASE(MY_WIFI_EVENT);

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_PROV_DONE_BIT BIT0

esp_event_loop_handle_t wifi_event_loop;

static const char *TAG = "WIFI_COMPONENT";

nvs_handle_t *wifi_nvs_hnd_ptr = NULL;
char *mqtt_broker_url; // variable para guardar el nombre introducido como custom_data

static int s_retry_num = 0;

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_SOFTAP "softap"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

void wifi_power_save(wifi_ps_type_t modo)
{
    esp_wifi_set_ps(modo);
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 * Note that memory for the response buffer must be allocated using heap as this buffer
 * gets freed by the protocomm layer once it has been sent by the transport layer.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{

    if (inbuf)
    {
        esp_err_t errcode = ESP_OK;

        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
        mqtt_broker_url = malloc(inlen + 1); // Se guardan los datos añadidos como custom_data en la variable nombre_dispositivo
        memcpy(mqtt_broker_url, inbuf, inlen);
        mqtt_broker_url[inlen] = '\0';
        errcode = nvs_set_str((*wifi_nvs_hnd_ptr), "MQTT_BROKER_URL", mqtt_broker_url);
        if (errcode != ESP_OK)
            ESP_LOGE(TAG, "Error writing Broker URL on NVS. %s", esp_err_to_name(errcode));
        else
        {
            if ((errcode = nvs_commit((*wifi_nvs_hnd_ptr))) == ESP_OK)
                ESP_LOGI(TAG, "Broker URL correctly saved on NVS");
            else
                ESP_LOGE(TAG, "Error commiting Broker URL on NVS. %s.", esp_err_to_name(errcode));
        }
        free(mqtt_broker_url);
    }

    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

esp_err_t mqtt_port_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                      uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    esp_err_t errcode = ESP_OK;
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
        int port = atoi((char *)inbuf);
        errcode = nvs_set_i32((*wifi_nvs_hnd_ptr), "MQTT_PORT", port);
        if (errcode != ESP_OK)
            ESP_LOGE(TAG, "Error writing Broker PORT on NVS. %s", esp_err_to_name(errcode));
        else
        {
            if ((errcode = nvs_commit((*wifi_nvs_hnd_ptr))) == ESP_OK)
                ESP_LOGI(TAG, "Broker PORT correctly saved on NVS");
            else
                ESP_LOGE(TAG, "Error commiting Broker URL on NVS. %s.", esp_err_to_name(errcode));
        }
    }

    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

esp_err_t mqtt_topic_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                       uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{

    esp_err_t errcode = ESP_OK;
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);

        char *topic = malloc(sizeof(char) * inlen + 1);
        memcpy(topic, inbuf, inlen);
        topic[inlen] = '\0';
        errcode = nvs_set_str((*wifi_nvs_hnd_ptr), "MQTT_TOPIC", topic) ;
        if (errcode!= ESP_OK)
            ESP_LOGE(TAG, "Error writing Device TOPIC on NVS. %s", esp_err_to_name(errcode));
        else
        {
            if ((errcode = nvs_commit((*wifi_nvs_hnd_ptr))) == ESP_OK)
                ESP_LOGI(TAG, "Broker URL correctly saved on NVS");
            else
                ESP_LOGE(TAG, "Error commiting Device TOPIC on NVS. %s.", esp_err_to_name(errcode));
        }
        free(topic);
    }

    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

static void wifi_prov_print_info(const char *name, const char *pop, const char *transport)
{
    ESP_LOGI(TAG, "Provisioning started using %s.", transport);
    ESP_LOGI(TAG, "Connect to SSID: %s", name);
    ESP_LOGI(TAG, "Proof of Possession (POP): %s", pop);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
    static int retries;
#endif
    if (event_base == WIFI_PROV_EVENT) // Provisionamiento WIFI SoftAP
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGE(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG,
                     "Received Wi-Fi credentials"
                     "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason =
                (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG,
                     "Provisioning failed!\n\tReason : %s"
                     "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR)
                         ? "Wi-Fi station authentication failed"
                         : "Wi-Fi access-point not found");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
            retries++;
            if (retries >= CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT)
            {
                ESP_LOGI(TAG, "Failed to connect with provisioned AP, resetting "
                              "provisioned credentials");
                wifi_prov_mgr_reset_sm_state_on_failure();
                retries = 0;
            }
#endif
            break;
        }
        case WIFI_PROV_CRED_SUCCESS: // Provisionamiento realizado correctamente
            ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_EXAMPLE_RESET_PROV_MGR_ON_FAILURE
            retries = 0;
#endif
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            // // Notificar a la funcion de que el provisionamiento termino y que pueda retornar
            // xEventGroupSetBits(s_wifi_event_group, WIFI_PROV_DONE_BIT);
            break;
        default:
            break;
        }
    }
    else if (event_base == WIFI_EVENT) // Eventos del modulo WiFi
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START: // Modo STA WiFi inicializado
            ESP_LOGE(TAG, "Evento WIFI_EVENT_STA_START recibido");
            esp_wifi_connect(); // Si no conecta a la primera, genera el evento de
                                // STA_DISCONNECTED
            break;
        case WIFI_EVENT_STA_CONNECTED: // Modo STA WiFi conectado
            ESP_LOGE(TAG, "Evento WIFI_EVENT_STA_CONNECTED recibido");
            // Notificar a la funcion de que estamos conectados y puede retornar
            xEventGroupSetBits(s_wifi_event_group, WIFI_PROV_DONE_BIT);
            break;
        case WIFI_EVENT_STA_DISCONNECTED: // Modo STA WiFi desconectado,
                                          // reintentamos conexion
            ESP_LOGE(TAG, "Evento WIFI_EVENT_STA_DISCONNECTED recibido");
            if (s_retry_num < CONFIG_WIFI_MAXIMUM_RETRY)
            {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            }
            else
                esp_event_post(MY_WIFI_EVENT, MY_WIFI_DISCONNECTED_EVENT, NULL, 0, portMAX_DELAY);

            break;
        case WIFI_EVENT_AP_STACONNECTED: // Dispositivo de provisionamiento
                                         // conectado
            ESP_LOGE(TAG, "Evento WIFI_EVENT_AP_STACONNECTED recibido");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED: // Dispositivo de provisionamiento
                                            // desconectado
            ESP_LOGE(TAG, "Evento WIFI_EVENT_AP_STADISCONNECTED recibido");

            break;
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP) // Evento de conexión, tenemos IP
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGE(TAG, "Connected with IP Address:" IPSTR,
                 IP2STR(&event->ip_info.ip));
        wifi_power_save(WIFI_PS_MAX_MODEM);
        s_retry_num = 0;

        esp_event_post(MY_WIFI_EVENT, MY_WIFI_CONNECTED_WITH_IP_EVENT, NULL, 0, portMAX_DELAY);
    }
    else if (event_base ==
             PROTOCOMM_SECURITY_SESSION_EVENT) // Eventos de seguridad de la
                                               // capa de provisionamiento
    {
        switch (event_id)
        {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters for establishing "
                          "secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing "
                          "secure session!");
            break;
        default:
            break;
        }
    }
}

esp_err_t wifi_init(nvs_handle_t *nvs_hnd)
{
    esp_err_t err;

    wifi_nvs_hnd_ptr = nvs_hnd; // Get NVS pointer to save ThingsBoard MQTT Broker URL when provisioned

    // Crear el event-group para sincronizar cuando acaba el provisionamiento con el retorno de la funcion wifi_init()
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    /* Initialize TCP/IP */
    err = esp_netif_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error in esp_netif_init: %s", esp_err_to_name(err));
        return err;
    }

    esp_event_loop_args_t ev_loop_args = {
        .queue_size = 5,
        .task_name = "wifi_events_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 3072,
        .task_core_id = tskNO_AFFINITY,
    };

    // Crear event loop
    err = esp_event_loop_create(&ev_loop_args, &wifi_event_loop);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error creando event_loop para los eventos de WIFI. %s", esp_err_to_name(err));
        return err;
    }

    if ((err = esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID,
                                          &wifi_event_handler, NULL)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error registrando eventos de provisionamiento WIFI: %s", esp_err_to_name(err));
        return err;
    }
    if ((err = esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT,
                                          ESP_EVENT_ANY_ID, &wifi_event_handler,
                                          NULL)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error registrando eventos de sesion segura en WIFI: %s", esp_err_to_name(err));
        return err;
    }
    if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          &wifi_event_handler, NULL)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error registrando eventos del modulo WIFI: %s", esp_err_to_name(err));
        return err;
    }
    if ((err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          &wifi_event_handler, NULL)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error registrando eventos del modulo IP_WIFI: %s", esp_err_to_name(err));
        return err;
    }

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if ((err = esp_wifi_init(&cfg)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error in esp_wifi_init: %s", esp_err_to_name(err));
        return err;
    }

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE};

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    if ((err = wifi_prov_mgr_init(config)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error in wifi_prov_mgr_init: %s", esp_err_to_name(err));
        return err;
    }

    bool provisioned = false;
    /* Let's find out if the device is provisioned */
    if ((err = wifi_prov_mgr_is_provisioned(&provisioned)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error in wifi_prov_mgr_is_provisioned: %s", esp_err_to_name(err));
        return err;
    }
    /* If device is not yet provisioned start provisioning service */
    if (!provisioned)
    {
        ESP_LOGI(TAG, "Starting provisioning");

        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        const char *pop = "abcd1234";

        wifi_prov_security1_params_t *sec_params = pop;

        const char *service_key = NULL;

        wifi_prov_mgr_endpoint_create("custom-data");
        wifi_prov_mgr_endpoint_create("mqtt-port");

        /* Start provisioning service */
        wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key);

        ESP_ERROR_CHECK(wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL));
        ESP_ERROR_CHECK(wifi_prov_mgr_endpoint_register("mqtt-port", mqtt_port_prov_data_handler, NULL));
        ESP_ERROR_CHECK(wifi_prov_mgr_endpoint_register("mqtt-topic", mqtt_topic_prov_data_handler, NULL));

        wifi_prov_print_info(service_name, pop, PROV_TRANSPORT_SOFTAP);

        // Esperamos hasta que termine el provisionamiento
        xEventGroupWaitBits(s_wifi_event_group, WIFI_PROV_DONE_BIT,
                            pdTRUE,         // limpiar el bit al retornar
                            pdFALSE,        // esperar cualquier bit
                            portMAX_DELAY); // bloquea indefinidamente
    }
    else
    {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        wifi_prov_mgr_deinit();

        // Saco las funciones de inicializacion del modo STA de la función porque daba fallos
        if ((err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK)
        {
            ESP_LOGE(TAG, "Error in esp_wifi_set_mode(WIFI_MODE_STA): %s", esp_err_to_name(err));
            return err;
        }
        if ((err = esp_wifi_start()) != ESP_OK)
        {
            ESP_LOGE(TAG, "Error in esp_wifi_start: %s", esp_err_to_name(err));
            return err;
        }
        // La llamada a esp_wifi_connect() se hace desde el event_handler del main, tras el evento WIFI_EVENT_STA_START

        // Esperamos hasta que nos indiquen que estamos conectados para retornar
        xEventGroupWaitBits(s_wifi_event_group, WIFI_PROV_DONE_BIT,
                            pdTRUE,         // limpiar el bit al retornar
                            pdFALSE,        // esperar cualquier bit
                            portMAX_DELAY); // bloquea indefinidamente
    }
    return err;
}
