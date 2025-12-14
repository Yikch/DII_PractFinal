idf_component_register(SRCS "green_led.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "driver" "esp_common" "log")