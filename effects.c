#include "effects.h"
#include "hsl_rgb.h"
#include <Arduino.h>
#include <math.h>

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
	float segmentStart;
	float segmentRadius;
} firesegment_t;

firesegment_t segments[1];

uint16_t randomMinMax(uint16_t min, uint16_t max) {
	return min + (rand() % (max+1-min));
}

firesegment_t randomFireSeg(uint16_t fadetime) {
	firesegment_t out;
	out.msStart = millis();
	out.msLengthTemperature = randomMinMax(300, fadetime);
	out.msLengthBlack = randomMinMax(100, fadetime);
	out.temperatureHue = ((float)randomMinMax(1, 7)) / 100;  // 1-7%
	out.segmentStart = ((float)randomMinMax(0, 100)) / 100;  // 0-100%;
	out.segmentRadius = ((float)randomMinMax(10, 30)) / 100;  // 10-30%;
	return out;
}

float levelAtSegment(uint8_t pixel, uint8_t length, float segmentStart, float segmentRadius) {
	float levelOut = 0.0;
	// Convert segment values (0.0 to 1.0) to pixel value (if length is 6; 0.0 to 6.0)
	segmentStart = segmentStart * length;
	segmentRadius = segmentRadius * length;
	float climbPerPixel = 1.0 / segmentRadius;
	// Offset segment by a length to avoid calculating with modula and if-statements
	float segmentMid = segmentStart + length;
	float segmentFirst = segmentMid - segmentRadius;
	float segmentLast = segmentMid + segmentRadius;
	
	// Test pixel if segment stretches before length, on length and after length
	for (uint8_t i = 0; i < 3; i++) {
		float level = 0.0;
		float currentPixel = (i*length) + pixel;
		if ((currentPixel >= segmentFirst) && (currentPixel <= segmentMid)) {
			// Up fade
			float diff = currentPixel - segmentFirst;
			level = diff * climbPerPixel;
		} else if ((currentPixel >= segmentMid) && (currentPixel <= segmentLast)) {
			// Down fade
			float diff = segmentLast - currentPixel;
			level = diff * climbPerPixel;
		}
		if (level > levelOut) {
			levelOut = level;
		}
	}
	return levelOut;
}

void fire(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, uint16_t fadetime) {
	unsigned long now = millis();

	// Go over all pixels
	for (uint8_t i = 0; i < length; i++) {
		// Reset to black pixels
		pixelbuffer[start+i].color = 0;

		// Go through fire segments and sum overlapping segments to current pixel
		float currentHue = 0;
		float currentLevel = 0;
		for (uint32_t s = 0; s < (sizeof segments / sizeof segments[0]); s++) {
			// If segment is over, generate a new one
			if (now > (segments[s].msStart + segments[s].msLengthTemperature + segments[s].msLengthBlack)) {
				segments[s] = randomFireSeg(fadetime);
			}
			uint16_t diff = now - segments[s].msStart;
			
			if (now < (segments[s].msStart + segments[s].msLengthTemperature)) {
				// Temperature part
				float hueSweepZero = 1.0 - (((float)diff) / ((float)segments[s].msLengthTemperature));  // 0.0-1.0 Sweep hue towards red color over segments duration
				float segmentLevel = levelAtSegment(i, length, segments[s].segmentStart, segments[s].segmentRadius);  // 0.0-1.0 Level over segment

				if (segmentLevel > 0) {
					currentHue = segments[s].temperatureHue * hueSweepZero;
					currentLevel = segmentLevel / 2;
				}
			} else {
				// Black part
				currentHue = 0;
				currentLevel = 0;
			}
		}
		// Add flicker???
		// Add remaining to white if hue is oversatureated???
		rgbw_t newPixel;
		newPixel = hslToRgb(currentHue, 1.0, currentLevel);
		pixelbuffer[start+i] = newPixel;
	}
}
