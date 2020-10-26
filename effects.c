#include "effects.h"
#include "hsl_rgb.h"
#include <Arduino.h>

void fade(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float hue, float range, uint16_t fadetime) {
	unsigned long now = millis();
	float step = range / length;  // Stretch across length pixels
	float fadeOffset;
	// Modulus fadetime and multipy it by range
	if ((now % fadetime) < (fadetime / 2)) {
		// First half of fade
		fadeOffset = (now % fadetime) * (range/fadetime);
	} else {
		// Second half of fade
		fadeOffset = (range/2) - (((now % fadetime) * (range/fadetime)) - (range/2));
	}
	for (uint32_t i = 0; i < length; i++) {
		pixelbuffer[start+i] = hslToRgb(hue+(i*step)+fadeOffset, 1.0, 0.5);
	}
}
