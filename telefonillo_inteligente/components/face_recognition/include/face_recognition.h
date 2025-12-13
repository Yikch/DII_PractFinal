#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void face_recognition_init(void);
    float recognize_face(void *buf, size_t buf_len);
#ifdef __cplusplus
}
#endif