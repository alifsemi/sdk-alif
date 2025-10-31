#ifndef IMAGE_ENSEMBLE_H
#define IMAGE_ENSEMBLE_H

#include <stdint.h>


#ifdef __cplusplus
 extern "C" {
#endif
int image_init();
const uint8_t *get_image_data(int ml_width, int ml_height);
#ifdef __cplusplus
}
#endif
#endif
