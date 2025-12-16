#include "lcd_s3.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_jpeg_dec.h"
#include "esp_heap_caps.h"
#include "jpeg_decoder.h"

static const char *TAG = "LCD_EYE";

// --- PINOUT OFICIAL ESP32-S3-EYE-SUB ---
#define LCD_HOST         SPI2_HOST
#define PIN_NUM_LCD_PCLK 21
#define PIN_NUM_LCD_DATA 47
#define PIN_NUM_LCD_CS   44
#define PIN_NUM_LCD_DC   43
#define PIN_NUM_LCD_RST  -1
#define PIN_NUM_LCD_BL   48 

// AJUSTE 1: Bajamos la frecuencia a 20MHz para descartar problemas de cableado/ruido
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)

static esp_lcd_panel_handle_t panel_handle = NULL;

esp_err_t lcd_s3_eye_init(void)
{
    ESP_LOGI(TAG, "1. Configurando Pines...");

    // --- AJUSTE 2: Resetear explícitamente el pin del Backlight ---
    gpio_reset_pin(PIN_NUM_LCD_BL);
    gpio_set_direction(PIN_NUM_LCD_BL, GPIO_MODE_OUTPUT);
    // Señal invertido, conectado a la puerta de un Mosfet de canal P
    gpio_set_level(PIN_NUM_LCD_BL, 1); // Apagado al principio para evitar chispazos visuales

    // Configurar Bus SPI
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_LCD_PCLK,
        .mosi_io_num = PIN_NUM_LCD_DATA,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        // Buffer para el "chunk" más grande que vayamos a enviar + cabeceras
        .max_transfer_sz = (LCD_WIDTH * 40 * 2) + 100 
    };
    
    // Usamos DMA Canal Auto
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Configurar Panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_LCD_DC,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ, // Frecuencia reducida
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    // Configurar Driver ST7789
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Resetear e Iniciar
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Configuración IPS
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    
    // Encender el Display (Comandos internos del chip)
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // --- AJUSTE 3: Encendido Físico del Backlight ---
    // Le damos un respiro de 100ms al panel antes de encender la luz trasera
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "Encendiendo Backlight (GPIO 48)...");
    gpio_set_level(PIN_NUM_LCD_BL, 0);

    return ESP_OK;
}

void lcd_s3_eye_draw_frame(const void *buffer)
{
    if (panel_handle == NULL) return;

    // Hacemos un cast a uint8_t para manipular bytes
    uint8_t *my_buf = (uint8_t *)buffer;
    
    // --- PARCHE SOFTWARE: SWAP DE BYTES ---
    // Invertimos cada pareja de bytes (Low <-> High)
    // OJO: Esto modifica el buffer de la cámara directamente.
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT * 2; i += 2) {
        uint8_t temp = my_buf[i];
        my_buf[i] = my_buf[i+1];
        my_buf[i+1] = temp;
    }
    // ---------------------------------------

    // Enviamos por bloques...
    uint16_t *p_buffer = (uint16_t *)my_buf;
    for (int y = 0; y < LCD_HEIGHT; y += 40) {
         // ... (resto del código de envío por bloques igual que antes)
         int lines_to_send = 40;
         if (y + lines_to_send > LCD_HEIGHT) lines_to_send = LCD_HEIGHT - y;
         
         esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_WIDTH, y + lines_to_send, p_buffer + (y * LCD_WIDTH));
    }
}

esp_err_t lcd_s3_eye_draw_jpeg(const uint8_t *jpg_data, size_t jpg_len)
{
    esp_err_t ret = ESP_OK;

    // 1. ASIGNAR MEMORIA PARA LA SALIDA (RGB565)
    // Calculamos el tamaño máximo necesario: Ancho * Alto * 2 bytes por píxel.
    // Usamos PSRAM (SPIRAM) porque ~115KB es mucho para la RAM interna.
    size_t out_buffer_size = LCD_WIDTH * LCD_HEIGHT * 2;
    uint8_t *out_buffer = (uint8_t *)heap_caps_malloc(out_buffer_size, MALLOC_CAP_SPIRAM);

    if (out_buffer == NULL) {
        ESP_LOGE(TAG, "No hay memoria suficiente para descomprimir el JPEG");
        return ESP_ERR_NO_MEM;
    }

    // 2. CONFIGURACIÓN DEL DECODIFICADOR
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)jpg_data,
        .indata_size = jpg_len,
        // --- CAMBIO CLAVE: Pasamos el buffer que acabamos de crear ---
        .outbuf = out_buffer,       
        .outbuf_size = out_buffer_size,
        // ------------------------------------------------------------
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 0,
        }
    };

    esp_jpeg_image_output_t outimg;

    // 3. DECODIFICAR
    // El decodificador leerá 'indata' y escribirá los píxeles en 'outbuf' (nuestro out_buffer)
    ret = esp_jpeg_decode(&jpeg_cfg, &outimg);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al decodificar JPEG: %d", ret);
        heap_caps_free(out_buffer); // Importante: liberar si falla
        return ret;
    }

    // 4. VERIFICACIÓN DE TAMAÑO
    // 'outimg' solo nos dice cuánto ocupó la imagen real
    if (outimg.width > LCD_WIDTH || outimg.height > LCD_HEIGHT) {
        ESP_LOGW(TAG, "Imagen JPEG (%dx%d) excede pantalla", outimg.width, outimg.height);
    }

    // 5. PINTAR EN LCD
    // Usamos 'out_buffer' que es donde están los datos crudos ahora.
    // NO usamos outimg.data (porque no existe en el struct).
    lcd_s3_eye_draw_frame(out_buffer);

    // 6. LIMPIEZA
    // Liberamos el buffer que nosotros mismos creamos al principio
    heap_caps_free(out_buffer);

    return ESP_OK;
}