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

typedef struct {
	unsigned long msStart;
	uint16_t msLengthTemperature;
	uint16_t msLengthBlack;
	float temperatureHue;
	uint8_t pxStart;
	uint8_t pxLength;
} firesegment_t;

firesegment_t segments[1];

uint16_t randomMinMax(uint16_t min, uint16_t max) {
	return min + (rand() % (max+1-min));
}

firesegment_t randomFireSeg(uint8_t length, uint16_t fadetime) {
	firesegment_t out;
	out.msStart = millis();
	out.msLengthTemperature = randomMinMax(300, fadetime);
	out.msLengthBlack = randomMinMax(100, fadetime);
	out.temperatureHue = ((float)randomMinMax(1, 9)) / 100;
	// Avoid px if it's occupied ???
	out.pxStart = randomMinMax(0, length-1);
	out.pxLength = randomMinMax(1, 3);
	return out;
}

rgbw_t highestRgb(rgbw_t a, rgbw_t b) {
	if ((a.red+a.green+a.blue) > (b.red+b.green+b.blue)) {
		return a;
	}  else {
		return b;
	}
}

rgbw_t lowestRgb(rgbw_t a, rgbw_t b) {
	if ((a.red+a.green+a.blue) < (b.red+b.green+b.blue)) {
		return a;
	}  else {
		return b;
	}
}

rgbw_t sumRgb(rgbw_t a, rgbw_t b) {
	rgbw_t out;
	out.red = max(a.red+b.red, 255);
	out.green = max(a.green+b.green, 255);
	out.blue = max(a.blue+b.blue, 255);
	return out;
}

void fire(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, uint16_t fadetime) {
	unsigned long now = millis();

	// Reset to black pixels
	for (uint8_t i = 0; i < length; i++) {
		pixelbuffer[start+i].color = 0;
	}

	// Go through fire segments and add them on top od LED buffer
	for (uint32_t s = 0; s < (sizeof segments / sizeof segments[0]); s++) {
		// If segment is over, generate a new one
		if (now > (segments[s].msStart + segments[s].msLengthTemperature + segments[s].msLengthBlack)) {
			segments[s] = randomFireSeg(length, fadetime);
		}
		
		// Go over each pixel in segment
		for (uint8_t p = 0; p < segments[s].pxLength; p++) {
			uint8_t px = start + ((segments[s].pxStart + p) % length);
			// Check if time is temperature or black part
			rgbw_t newPixel;
			if (now < (segments[s].msStart + segments[s].msLengthTemperature)) {
				// Temperature part
				uint16_t diff = now - segments[s].msStart;
				// Sweep hue towards red color over segments duration
				float huePercent = 1.0 - (((float)diff) / ((float)segments[s].msLengthTemperature));
				// Fade out segment at last 20% of duration
				float lightnessPercent = 0.5;
				if (huePercent < 0.20) {
					lightnessPercent = lightnessPercent * (huePercent*5);
				}
				// Add flicker???
				newPixel = hslToRgb(0.0+(segments[s].temperatureHue*huePercent), 1.0, lightnessPercent);
			} else {
				// Black part
				newPixel.color = 0;
			}
			// pixelbuffer[px] = highestRgb(newPixel, pixelbuffer[px]);
			pixelbuffer[px] = newPixel;
		}
	}
}
