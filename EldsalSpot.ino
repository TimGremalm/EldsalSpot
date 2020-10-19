// The serial port is ised for DMX transmit normally. If you want debug-print define USE_PRINT.
//#define USE_PRINT

#ifndef USE_PRINT
#include <DMXSerial.h>
#endif

// 0-10V light control signals
const int pinRoofLevel = 1;
const int pinRoofMode = 2;
const int pinSpotsLevel = 3;
const int pinSpotsLevel = 4;


// Parcan channels
const int spotDmxChannelRed = 1;
const int spotDmxChannelGreen = 2;
const int spotDmxChannelBlue = 3;
const int spotDmxChannelWhite = 4;
const int spotDmxChannelDimmer = 5;
const int spotDmxChannelStrobe = 6;

// Number of par cans
const int roofNumberOfParcans = 6;
const int spotsNumberOfSpots = 6;

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	#ifdef USE_PRINT
	Serial.begin(115200);
	Serial.println("Start EldsalSpot");
	#else
	DMXSerial.init(DMXController);
	#endif
}

void checkLevels() {
	int pot1 = analogRead(A1);
	pot1 = map(pot1, 0, 1023, 0, 255); // Scale 0-10V input to
	pot1 = max(pot1, 0); // Avoid negative numbers
	smokeOnTime = (float)pot1 / 255;
}

void tick() {
	#ifndef USE_PRINT
	DMXSerial.write(1, smokeLevel);
	#endif

	#ifdef USE_PRINT
	Serial.print("On time: ");
	Serial.println(secondsSinceSmokeToggled);
	#endif
}

void loop() {
	checkLevels();
	tick();
	delay(100);
}
