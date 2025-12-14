#include "esp_log.h"
#include "human_face_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "bsp/esp-bsp.h"
#include "face_recognition.h"

const char *TAG = "face_recognition";
static HumanFaceDetect *detect = nullptr;

esp_err_t face_recognition_init(void)
{
    if (detect != nullptr)
    {
        // Ya inicializado, solo reportar éxito.
        ESP_LOGW(TAG, "Detector ya inicializado.");
        return ESP_OK;
    }

    detect = new HumanFaceDetect();

    if (detect == nullptr)
    {
        ESP_LOGE(TAG, "ERROR: Falló la asignación de memoria para HumanFaceDetect.");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Detector de Rostros inicializado exitosamente.");

    return ESP_OK;
}

float recognize_face(void *buf, size_t buf_len)
{
    dl::image::jpeg_img_t jpeg_img = {
        .data = buf,
        .data_len = buf_len};

    // DECODIFICACIÓN DE LA IMAGEN (Asigna memoria)
    auto img = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    float score = -1.0f;

    ESP_LOGI(TAG, "img: width=%d height=%d pix_type=%d data=%p",
             img.width, img.height, img.pix_type, img.data);

    // 1. Manejo de error de decodificación
    if (!img.data)
    {
        ESP_LOGE(TAG, "ERROR: Fallo al decodificar la imagen JPEG.");
        return 0.0f;
    }

    // 2. Ejecución del método C++
    auto &detect_results = detect->run(img);

    if (detect_results.empty())
    {
        ESP_LOGI(TAG, "No se detectaron rostros.");
    }
    else
    {
        const dl::detect::result_t *best = nullptr;
        //ESP_LOGI(TAG, "No se detectaron rostros.");

        for (auto &res : detect_results)
        {
            if (!best || res.score > best->score)
            {
                best = &res;
            }
        }

        if (best)
        {
            score = best->score;
        }
    }

    // 3. Limpieza de memoria (Garantizada)
    heap_caps_free(img.data);

    return score;
}