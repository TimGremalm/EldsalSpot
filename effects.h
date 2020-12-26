#ifdef __cplusplus
extern "C" {
#endif

#ifndef __EFFECTS__
#define __EFFECTS__

#include "hsl_rgb.h"

typedef enum {
	LIGHT_FADE_WHITE = 0,
	LIGHT_FADE_WHITE2,
	LIGHT_FADE_RED,
	LIGHT_FADE_MAGENTA,
	LIGHT_FADE_BLUE,
	LIGHT_FADE_CYAN,
	LIGHT_FADE_GREEN,
	LIGHT_FADE_YELLOW,
	LIGHT_FADE_YELLOW_RED,
	LIGHT_FADE_CYAN_YELLOW,
	LIGHT_FADE_MAGENTA_BLUE,
	LIGHT_FIRE_SLOW,
	LIGHT_FIRE_FAST,
	LIGHT_PAR_FLASH_RED,
	LIGHT_PAR_FLASH_MAGENTA,
	LIGHT_PAR_FLASH_BLUE,
	LIGHT_PAR_FLASH_CYAN,
	LIGHT_PAR_FLASH_GREEN,
	LIGHT_PAR_FLASH_YELLOW,
	LIGHT_POLKA,
	LIGHT_CANDY,
	LIGHT_RAINBOW_SLOW,
	LIGHT_RAINBOW_FAST,
	LIGHT_START = 255,
} light_mode_t;

void fade(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float hue, float range, uint16_t fadetime);
void white(rgbw_t *pixelbuffer, uint8_t start, uint8_t length);
void fire(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, uint16_t fadetime);
void flash(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, float bpm, float beatMultiple, float fadePart, float pairOffset, float colorA, float colorB, float colorShiftBeat, float colorShiftAmount);
void sweepColors(rgbw_t *pixelbuffer, uint8_t start, uint8_t length, rgbw_t *colors, uint8_t numColors, uint16_t msPerColor);

#endif  // __EFFECTS__

#ifdef __cplusplus
}
#endif
