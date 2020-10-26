#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HSL_RGB__
#define __HSL_RGB__

#include <stdint.h>

typedef union {
	struct {
		uint8_t blue; //LSB
		uint8_t green;
		uint8_t red;
		uint8_t white;
	};
	uint32_t color; // 0xWWRRGGBB
} rgbw_t;

rgbw_t hslToRgb(float h, float s, float l);

#endif  // __HSL_RGB__

#ifdef __cplusplus
}
#endif
