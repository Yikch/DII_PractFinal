idf_component_register(SRCS "cliente_mqtt.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt nvs_flash json)
