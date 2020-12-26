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

void white(rgbw_t *pixelbuffer, uint8_t start, uint8_t length) {
	rgbw_t newPixel;
	newPixel = hslToRgb(0.10, 1.0, 0.1);
	newPixel.white = 255;
	for (uint32_t i = 0; i < length; i++) {
		pixelbuffer[start+i] = newPixel;
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

firesegment_t segments[5];

uint16_t randomMinMax(uint16_t min, uint16_t max) {
	return min + (rand() % (max+1-min));
}

firesegment_t randomFireSeg(uint16_t fadetime) {
	firesegment_t out;
	out.msStart = millis();
	out.msLengthTemperature = randomMinMax((int)(fadetime*0.4), (int)(fadetime*1.2));
	out.msLengthBlack = randomMinMax((int)(fadetime*0.0), (int)(fadetime*0.0));
	out.temperatureHue = ((float)randomMinMax(0, 7)) / 100;  // 1-7%
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
				float fadeAtEnd = 1.0;
				if (hueSweepZero < 0.20) {
					fadeAtEnd = hueSweepZero * 5;
				}

				if (segmentLevel > 0) {
					// Accumulate hue from all the segments 
					currentHue = currentHue + (segments[s].temperatureHue * hueSweepZero);
					if ((0.5 * segmentLevel * fadeAtEnd) > currentLevel) {
						currentLevel = 0.5 * segmentLevel * fadeAtEnd;
					}
					//currentLevel = 0.5 * segmentLevel * fadeAtEnd;
				}
			} else {
				// Black part
				// currentHue = 0;
				// currentLevel = 0;
			}
		}
		// Add flicker???
		// Add remainder of hue to white if oversatureated
		float white = 0;
		float cutoff = 0.06;
		float cutoff_white = 0.20;
		if (currentHue > cutoff) {
			white = currentHue - cutoff;
			if (white > cutoff_white) {
				white = cutoff_white;
			}
			currentHue = cutoff;
		}
		rgbw_t newPixel;
		newPixel = hslToRgb(currentHue, 1.0, currentLevel);
		newPixel.white = white;
		newPixel.blue = 0;
		pixelbuffer[start+i] = newPixel;
	}
}

float calcFade(float beatPart, float fadeLimitUp, float fadeLimitDown) {
	float out;
	if (beatPart <= fadeLimitUp) {
		// First part; fade up
		out = beatPart / fadeLimitUp;
	} else if ((beatPart > fadeLimitUp) && (beatPart < fadeLimitDown)) {
		// Second part; fade down
		out = 1.0 - ((beatPart-fadeLimitUp) / fadeLimitUp);
	} else {
		// Black part
		out = 0.0;
	}
	return out;
}

void flash(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float bpm, float beatMultiple, float fadePart, float pairOffset, float colorA, float colorB, float colorShiftBeat, float colorShiftAmount) {
	unsigned long now = millis();
	uint16_t beatMs = (1.0 / (bpm/60)) * 1000 * beatMultiple;
	
	float fadeLimitUp = beatMs * fadePart;
	float fadeLimitDown = beatMs * (fadePart * 2);
	
	// Calculate fade for even pixels
	float beatPart = now % beatMs; // Modulus beatMs to get beat
	float fadeA = calcFade(beatPart, fadeLimitUp, fadeLimitDown);
	
	// Calculate fade for odd pixels
	beatPart = ((int)(beatPart + (beatMs * pairOffset))) % beatMs; // Ofset beat for odd pixels
	float fadeB = calcFade(beatPart, fadeLimitUp, fadeLimitDown);
	
	// Calculate color shift
	float beatColorPart = now % (beatMs * (int)colorShiftBeat);
	float beatColorLimitUp = beatMs * (int)colorShiftBeat;
	float beatColorLimitDown = beatMs * (int)colorShiftBeat * 2;
	float colorOffset = calcFade(beatColorPart, beatColorLimitUp, beatColorLimitDown);
	colorOffset = colorOffset * colorShiftAmount;
	
	// Write color values
	for (uint32_t i = 0; i < length; i++) {
		if ((i % 2) == 0) {
			// Even pixel
			pixelbuffer[start+i] = hslToRgb(colorA+colorOffset, 1.0, fadeA/2);
		} else {
			// Odd pixel
			pixelbuffer[start+i] = hslToRgb(colorB+colorOffset, 1.0, fadeB/2);
		}
	}
}

uint8_t gradient(uint8_t colorA, uint8_t colorB, float progress) {
	uint8_t diff;
	if (colorA > colorB) {
		diff = colorA - colorB;
		return colorB + (uint8_t)((1.0 - progress) * diff);
	} else {
		diff = colorB - colorA;
		return colorA + (uint8_t)(progress * diff);
	}
}

rgbw_t gradient_color(rgbw_t colorA, rgbw_t colorB, float progress) {
	rgbw_t out;
	out.red = gradient(colorA.red, colorB.red, progress);
	out.green = gradient(colorA.green, colorB.green, progress);
	out.blue = gradient(colorA.blue, colorB.blue, progress);
	out.white = gradient(colorA.white, colorB.white, progress);
	return out;
}

void sweepColors(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, rgbw_t *colors, uint8_t numColors, uint16_t msPerColor) {
	unsigned long now = millis();
	float mscounter = now % (numColors * msPerColor);
	uint8_t currentColor = mscounter / msPerColor;
	float current = (mscounter - (currentColor * msPerColor)) / msPerColor;
	for (uint8_t i = 0; i < length; i++) {
		pixelbuffer[start+i] = gradient_color(	colors[(currentColor+i+0) % numColors],
												colors[(currentColor+i+1) % numColors],
												current);
	}
}
