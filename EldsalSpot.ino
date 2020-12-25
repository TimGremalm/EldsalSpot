#include "hsl_rgb.h"
#include "effects.h"

// The serial port is used for DMX transmit normally. If you want debug-print define USE_PRINT.
// #define USE_PRINT

#include <Adafruit_NeoPixel.h>
#ifndef USE_PRINT
#include <DMXSerial.h>
#endif

// Parcan channels
const uint8_t spotDmxChannelStart = 1;
const uint8_t spotDmxChannels = 6;
const uint8_t spotDmxChannelRed = 0;
const uint8_t spotDmxChannelGreen = 1;
const uint8_t spotDmxChannelBlue = 2;
const uint8_t spotDmxChannelWhite = 3;
const uint8_t spotDmxChannelDimmer = 4;
const uint8_t spotDmxChannelStrobe = 5;

// Number of par cans
const uint8_t roofNumberOfParcans = 6;
const uint8_t spotsNumberOfSpots = 4;

const float fixedBpm = 123;

// Pixel buffer
rgbw_t grbwPixels[roofNumberOfParcans + spotsNumberOfSpots];

// 0-10V light control signals
const int pinRoofLevel = A0;
const int pinRoofMode = A1;
const int pinSpotsLevel = A2;
const int pinSpotsMode = A3;

// Mode parameters
const uint16_t modeSteps = 24;
const uint16_t modeStep = 1024 / modeSteps;
const uint16_t modeDiffLatch = modeStep / 2;

float roofLevel = 0;
float spotsLevel = 0;
light_mode_t roofMode = LIGHT_START;
light_mode_t spotsMode = LIGHT_START;

Adafruit_NeoPixel pixels(roofNumberOfParcans+spotsNumberOfSpots, 3, NEO_GRB + NEO_KHZ800);

void setup() {
	pixels.begin();
	#ifdef USE_PRINT
	Serial.begin(115200);
	Serial.println("Start EldsalSpot");
	#else
	DMXSerial.init(DMXController);
	#endif
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
			Serial.print(roofMode);
			Serial.print(" ");
			printMode(roofMode);
			Serial.println(" ");
			#endif
		}
		if (analogRoofMode < ((roofMode * (uint16_t)modeStep) - (modeDiffLatch * 1))) {
			roofMode = (light_mode_t)(analogRoofMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Roof new mode down ");
			Serial.print(roofMode);
			Serial.print(" ");
			printMode(roofMode);
			Serial.println(" ");
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
			Serial.print(spotsMode);
			Serial.print(" ");
			printMode(spotsMode);
			Serial.println(" ");
			#endif
		}
		if (analogSpotsMode < ((spotsMode * (uint16_t)modeStep) - (modeDiffLatch * 1))) {
			spotsMode = (light_mode_t)(analogSpotsMode / (uint16_t)modeStep);
			#ifdef USE_PRINT
			Serial.print("Spots new mode down ");
			Serial.print(spotsMode);
			Serial.print(" ");
			printMode(spotsMode);
			Serial.println(" ");
			#endif
		}
	}
}

void printMode(light_mode_t mode) {
	#ifdef USE_PRINT
	switch (mode) {
		case LIGHT_START: Serial.print("LIGHT_START"); break;
		case LIGHT_FADE_WHITE: Serial.print("LIGHT_FADE_WHITE"); break;
		case LIGHT_FADE_RED: Serial.print("LIGHT_FADE_RED"); break;
		case LIGHT_FADE_MAGENTA: Serial.print("LIGHT_FADE_MAGENTA"); break;
		case LIGHT_FADE_BLUE: Serial.print("LIGHT_FADE_BLUE"); break;
		case LIGHT_FADE_CYAN: Serial.print("LIGHT_FADE_CYAN"); break;
		case LIGHT_FADE_GREEN: Serial.print("LIGHT_FADE_GREEN"); break;
		case LIGHT_FADE_YELLOW: Serial.print("LIGHT_FADE_YELLOW"); break;
		case LIGHT_FADE_MAGENTA_BLUE: Serial.print("LIGHT_FADE_MAGENTA_BLUE"); break;
		case LIGHT_FADE_YELLOW_RED: Serial.print("LIGHT_FADE_YELLOW_RED"); break;
		case LIGHT_FADE_CYAN_YELLOW: Serial.print("LIGHT_FADE_CYAN_YELLOW"); break;
		case LIGHT_RAINBOW_SLOW: Serial.print("LIGHT_RAINBOW_SLOW"); break;
		case LIGHT_RAINBOW_FAST: Serial.print("LIGHT_RAINBOW_FAST"); break;
		case LIGHT_POLKA: Serial.print("LIGHT_POLKA"); break;
		case LIGHT_CANDY: Serial.print("LIGHT_CANDY"); break;
		case LIGHT_FIRE_SLOW: Serial.print("LIGHT_FIRE_SLOW"); break;
		case LIGHT_FIRE_FAST: Serial.print("LIGHT_FIRE_FAST"); break;
		case LIGHT_PAR_FLASH_RED: Serial.print("LIGHT_PAR_FLASH_RED"); break;
		case LIGHT_PAR_FLASH_MAGENTA: Serial.print("LIGHT_PAR_FLASH_MAGENTA"); break;
		case LIGHT_PAR_FLASH_BLUE: Serial.print("LIGHT_PAR_FLASH_BLUE"); break;
		case LIGHT_PAR_FLASH_CYAN: Serial.print("LIGHT_PAR_FLASH_CYAN"); break;
		case LIGHT_PAR_FLASH_GREEN: Serial.print("LIGHT_PAR_FLASH_GREEN"); break;
		case LIGHT_PAR_FLASH_YELLOW: Serial.print("LIGHT_PAR_FLASH_YELLOW"); break;
	}	
	#endif
}

void lightProgramByMode(light_mode_t mode, uint8_t start, uint8_t length) {
	switch (mode) {
		case LIGHT_START:
		case LIGHT_FADE_WHITE:
		case LIGHT_FADE_WHITE2:
			white(grbwPixels, start, length);
			break;
		case LIGHT_FADE_RED:
			fade(grbwPixels, start, length, 0.98, 0.02, 10000);
			break;
		case LIGHT_FADE_MAGENTA:
			fade(grbwPixels, start, length, 0.79, 0.15, 10000);
			break;
		case LIGHT_FADE_BLUE:
			fade(grbwPixels, start, length, 0.55, 0.10, 10000);
			break;
		case LIGHT_FADE_CYAN:
			fade(grbwPixels, start, length, 0.45, 0.15, 7000);
			break;
		case LIGHT_FADE_GREEN:
			fade(grbwPixels, start, length, 0.27, 0.05, 6000);
			break;
		case LIGHT_FADE_YELLOW:
			fade(grbwPixels, start, length, 0.06, 0.05, 6000);
			break;
		case LIGHT_FADE_YELLOW_RED:
			fade(grbwPixels, start, length, 0.99, 0.06, 12000);
			break;
		case LIGHT_FADE_CYAN_YELLOW:
			fade(grbwPixels, start, length, 0.85, 0.25, 18000);
			break;
		case LIGHT_FADE_MAGENTA_BLUE:
			fade(grbwPixels, start, length, 0.65, 0.25, 16000);
			break;
		case LIGHT_FIRE_SLOW:
			fire(grbwPixels, start, length, 1300);
			break;
		case LIGHT_FIRE_FAST:
			fire(grbwPixels, start, length, 600);
			break;
		case LIGHT_PAR_FLASH_RED:
			flash(grbwPixels, start, length, fixedBpm, 4, 0.25, 0.0, 0.0, 0.5, 2, 1.0);
			break;
		case LIGHT_PAR_FLASH_MAGENTA:
		case LIGHT_PAR_FLASH_BLUE:
		case LIGHT_PAR_FLASH_CYAN:
		case LIGHT_PAR_FLASH_GREEN:
		case LIGHT_PAR_FLASH_YELLOW:
			flash(grbwPixels, start, length, fixedBpm, 4, 0.25, 0.5, 0.0, 0.5, 1, 0);
			break;
		case LIGHT_POLKA:
		case LIGHT_CANDY:
		case LIGHT_RAINBOW_SLOW:
		case LIGHT_RAINBOW_FAST:
			break;
	}
}

void tick() {
	lightProgramByMode(roofMode, 0, roofNumberOfParcans);
	lightProgramByMode(spotsMode, roofNumberOfParcans, spotsNumberOfSpots);
	fillWs2812(grbwPixels);
}

void fillWs2812(rgbw_t *pixelbuffer) {
	for (uint32_t i = 0; i < (roofNumberOfParcans + spotsNumberOfSpots); i++) {
		pixels.setPixelColor(i, pixelbuffer[i].color);
		#ifndef USE_PRINT
		DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelRed, pixelbuffer[i].red);
		DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelGreen, pixelbuffer[i].green);
		DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelBlue, pixelbuffer[i].blue);
		DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelWhite, pixelbuffer[i].white);
		if (i < roofNumberOfParcans) {
			DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelDimmer, 255*roofLevel);
		} else {
			DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelDimmer, 255*spotsLevel);
		}
		DMXSerial.write(spotDmxChannelStart+(spotDmxChannels*i)+spotDmxChannelStrobe, 0);
		#endif
	}
	pixels.show();
}

void loop() {
	checkLevels();
	tick();
	delay(20);  // Cap to 50Hz, just around the limit of DMX512
}
