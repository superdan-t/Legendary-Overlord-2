#include <Dimmers.h>
#include <FastLED.h>

#define VER_MAJOR 3
#define VER_MINOR 0
#define VER_PATCH 0

#define EOT 4
#define DLE 16

#define SERIAL_MAX_SIZE 50

#define DATA_PIN 3
#define STRIP_LEN 150
#define XMAS_LEN 50
#define NUM_LEDS XMAS_LEN + STRIP_LEN

#define E_FILL 1

CRGB leds[NUM_LEDS];

struct Color {
  byte red = 0;
  byte green = 0;
  byte blue = 0;
};

struct Preset {
  Color color[150];
};
Preset rainbow;

byte xmasEffect = 0;
byte stripEffect = 0;

byte serialBuf[SERIAL_MAX_SIZE];
byte serialIndex = 0;

void setup() {

  Serial.begin(115200);

  //FastLED Setup
  LEDS.addLeds<WS2811,DATA_PIN,RGB>(leds,XMAS_LEN);
  LEDS.addLeds<WS2812,DATA_PIN,GRB>(&leds[XMAS_LEN], STRIP_LEN);
  LEDS.setBrightness(84);

  makeRainbow(&rainbow);
  
  fillEffect(&rainbow, 5, 10);
}

void loop() {

}
