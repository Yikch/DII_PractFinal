idf_component_register(SRCS "lcd_s3.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver esp_lcd esp_driver_spi esp_jpeg esp_new_jpeg)