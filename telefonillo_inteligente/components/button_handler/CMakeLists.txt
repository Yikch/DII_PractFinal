idf_component_register(SRCS "button_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_common esp_event freertos esp_adc)