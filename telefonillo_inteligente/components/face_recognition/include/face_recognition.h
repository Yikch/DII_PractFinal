#ifndef FACE_RECOGNITION_H
#define FACE_RECOGNITION_H
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t face_recognition_init(void);
    float recognize_face(void *buf, size_t buf_len);
#ifdef __cplusplus
}
#endif
#endif