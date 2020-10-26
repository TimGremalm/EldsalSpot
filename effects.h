#ifdef __cplusplus
extern "C" {
#endif

#ifndef __EFFECTS__
#define __EFFECTS__

#include "hsl_rgb.h"

void fade(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float hue, float range, uint16_t fadetime);

#endif  // __EFFECTS__

#ifdef __cplusplus
}
#endif
