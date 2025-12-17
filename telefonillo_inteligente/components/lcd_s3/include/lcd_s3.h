#ifndef LCD_S3_H_
#define LCD_S3_H_
#pragma once
#include "esp_err.h"

// Resolución nativa de la S3-EYE Sub Board
#define LCD_WIDTH   240
#define LCD_HEIGHT  240

/**
 * @brief Inicializa la pantalla ST7789 específica del ESP32-S3-EYE-SUB
 * Incluye configuración SPI, driver de panel y encendido de Backlight.
 */
esp_err_t lcd_s3_eye_init(void);


/**
 * @brief Decodifica un buffer JPEG y lo muestra en la pantalla
 * @param jpg_data Puntero al inicio del buffer JPEG (fb->buf)
 * @param jpg_len Longitud del buffer JPEG (fb->len)
 */
esp_err_t lcd_s3_eye_draw_jpeg(const uint8_t *jpg_data, size_t jpg_len);

#endif