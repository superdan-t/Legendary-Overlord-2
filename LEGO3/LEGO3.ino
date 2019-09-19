#include <Dimmers.h>
#include <FastLED.h>

#define VER_MAJOR 3
#define VER_MINOR 0
#define VER_PATCH 0

#define EOT 4
#define DLE 16

#define SERIAL_MAX_SIZE 50

#define DATA_PIN 3
#define XMAS_LEN 50
#define XMAS_START 150
#define STRIP_LEN 150
#define STRIP_START 0
#define TOTAL_LEN STRIP_LEN + XMAS_LEN

#define E_FILL 1

CRGB leds[TOTAL_LEN];

byte effectSpeed;

byte xmasEffect = 0;
byte stripEffect = 0;

byte serialBuf[SERIAL_MAX_SIZE];
byte serialIndex = 0;

typedef struct EffectController {
  CRGB (*generator)(byte, byte);
  bool (*effect)(EffectController*);
  unsigned long nextTick;
  byte data[10];
  byte threadID;
};

//This replaces 16 booleans. Use bitwise operations to access
unsigned int effectThreadStates = 0;

EffectController effectThreads[16];

void setup() {

  Serial.begin(115200);

  //FastLED Setup
  LEDS.addLeds<WS2812, DATA_PIN, GRB>(&leds[STRIP_START], STRIP_LEN);
  LEDS.addLeds<WS2811, DATA_PIN, RGB>(&leds[XMAS_START], XMAS_LEN);
  LEDS.setBrightness(255);

  for (byte i = 0; i < TOTAL_LEN; i++) {
    leds[i] = getPatriotic(i, TOTAL_LEN);
  }

  FastLED.show();
  while (true);

//  EffectController myNewEffect;
//  myNewEffect.generator = &getPatriotic;
//  myNewEffect.effect = &mySampleEffect;
//  myNewEffect.data[0] = 0;
//  myNewEffect.threadID = 200;
//
//  Serial.println(registerEffect(&myNewEffect));
//
//  myNewEffect.effect = &shiftForever;
//  myNewEffect.threadID = 202;
//
//  Serial.println(registerEffect(&myNewEffect));

}

void loop() {

  runEffectThreads();

}
