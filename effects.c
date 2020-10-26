#include "effects.h"
#include "hsl_rgb.h"

void fade(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float hue) {
	float step = 0.2 / length;
	for (uint32_t i = 0; i < length; i++) {
		pixelbuffer[start+i] = hslToRgb(hue+(i*step), 1.0, 0.5);
	}
}
