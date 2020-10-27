#include <Adafruit_NeoPixel.h>
#include "hsl_rgb.h"
#include "effects.h"

// The serial port is used for DMX transmit normally. If you want debug-print define USE_PRINT.
#define USE_PRINT

#ifndef USE_PRINT
// #include <DMXSerial.h>
#endif

// Parcan channels
const uint8_t spotDmxChannelRed = 1;
const uint8_t spotDmxChannelGreen = 2;
const uint8_t spotDmxChannelBlue = 3;
const uint8_t spotDmxChannelWhite = 4;
const uint8_t spotDmxChannelDimmer = 5;
const uint8_t spotDmxChannelStrobe = 6;

// Number of par cans
const uint8_t roofNumberOfParcans = 6;
const uint8_t spotsNumberOfSpots = 4;

// Pixel buffer
rgbw_t grbwPixels[roofNumberOfParcans + spotsNumberOfSpots];
//rgbw_t grbwPixels[10];

// 0-10V light control signals
const int pinRoofLevel = A0;
const int pinRoofMode = A1;
const int pinSpotsLevel = A2;
const int pinSpotsMode = A3;

// Mode parameters
const uint16_t modeSteps = 24;
const uint16_t modeStep = 1024 / modeSteps;
const uint16_t modeDiffLatch = modeStep / 2;
typedef enum {
	LIGHT_FADE_WHITE = 0,
	LIGHT_FADE_RED,
	LIGHT_FADE_MAGENTA,
	LIGHT_FADE_BLUE,
	LIGHT_FADE_CYAN,
	LIGHT_FADE_GREEN,
	LIGHT_FADE_YELLOW,
	LIGHT_FADE_MAGENTA_BLUE,
	LIGHT_FADE_YELLOW_RED,
	LIGHT_FADE_CYAN_YELLOW,
	LIGHT_RAINBOW_SLOW,
	LIGHT_RAINBOW_FAST,
	LIGHT_POLKA,
	LIGHT_CANDY,
	LIGHT_FIRE_SLOW,
	LIGHT_FIRE_FAST,
	LIGHT_PAR_FLASH_RED,
	LIGHT_PAR_FLASH_MAGENTA,
	LIGHT_PAR_FLASH_BLUE,
	LIGHT_PAR_FLASH_CYAN,
	LIGHT_PAR_FLASH_GREEN,
	LIGHT_PAR_FLASH_YELLOW,
	LIGHT_START = 255,
} light_mode_t;

float roofLevel = 0;
float spotsLevel = 0;
light_mode_t roofMode = LIGHT_START;
light_mode_t spotsMode = LIGHT_START;

Adafruit_NeoPixel pixels(roofNumberOfParcans+spotsNumberOfSpots, 3, NEO_GRB + NEO_KHZ800);

void setup() {
	#ifdef USE_PRINT
	Serial.begin(115200);
	Serial.println("Start EldsalSpot");
	#else
	DMXSerial.init(DMXController);
	#endif
	pixels.begin();
	memset(grbwPixels, 0, sizeof(rgbw_t) * (roofNumberOfParcans + spotsNumberOfSpots));
}

void checkLevels() {
	roofLevel = ((float)analogRead(pinRoofLevel)) / 1024;
	spotsLevel = ((float)analogRead(pinSpotsLevel)) / 1024;

	uint16_t analogRoofMode = analogRead(pinRoofMode);
	uint16_t analogSpotsMode = analogRead(pinSpotsMode);

	// Base mode of analog value
	if ((light_mode_t)(analogRoofMode / modeStep) != roofMode) {
		// Make sure we have stretched far enough in to the new mode
		if (analogRoofMode > ((roofMode * (uint16_t)modeStep) + (modeDiffLatch * 3))) {
			roofMode = (light_mode_t)(analogRoofMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Roof new mode up ");
			Serial.println(roofMode);
			#endif
		}
		if (analogRoofMode < ((roofMode * (uint16_t)modeStep) - (modeDiffLatch * 1))) {
			roofMode = (light_mode_t)(analogRoofMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Roof new mode down ");
			Serial.println(roofMode);
			#endif
		}
	}

	// Base mode of analog value
	if ((light_mode_t)(analogSpotsMode / modeStep) != spotsMode) {
		// Make sure we have stretched far enough in to the new mode
		if (analogSpotsMode > ((spotsMode * (uint16_t)modeStep) + (modeDiffLatch * 3))) {
			spotsMode = (light_mode_t)(analogSpotsMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Spots new mode up ");
			Serial.println(spotsMode);
			#endif
		}
		if (analogSpotsMode < ((spotsMode * (uint16_t)modeStep) - (modeDiffLatch * 1))) {
			spotsMode = (light_mode_t)(analogSpotsMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Spots new mode down ");
			Serial.println(spotsMode);
			#endif
		}
	}
}

void tick() {
	#ifndef USE_PRINT
	DMXSerial.write(1, smokeLevel);
	#endif

	switch (roofMode) {
		case LIGHT_START:
		case LIGHT_FADE_RED:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.98, 0.03, 10000);
			break;
		case LIGHT_FADE_MAGENTA:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.80, 0.10, 10000);
			break;
		case LIGHT_FADE_BLUE:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.60, 0.05, 10000);
			break;
		case LIGHT_FADE_CYAN:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.40, 0.10, 10000);
			break;
		case LIGHT_FADE_GREEN:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.30, 0.05, 10000);
			break;
		case LIGHT_FADE_YELLOW:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.10, 0.05, 10000);
			break;
		case LIGHT_FADE_YELLOW_RED:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.00, 0.10, 10000);
			break;
		case LIGHT_FADE_CYAN_YELLOW:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.85, 0.25, 10000);
			break;
		case LIGHT_FADE_MAGENTA_BLUE:
			fade(grbwPixels, 0, roofNumberOfParcans, 0.65, 0.25, 10000);
			break;
	}	
	fillWs2812(grbwPixels);
}

void fillWs2812(rgbw_t *pixelbuffer) {
	for (uint32_t i = 0; i < (roofNumberOfParcans + spotsNumberOfSpots); i++) {
		pixels.setPixelColor(i, pixelbuffer[i].color);
	}
	pixels.show();
}

void loop() {
	checkLevels();
	tick();
	delay(20);  // Cap to 50Hz, just around the limit of DMX512
}
