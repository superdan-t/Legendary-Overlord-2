#include <Dimmers.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FastLED.h>
#include <SD.h>

#define VER_MAJOR 3
#define VER_MINOR 0
#define VER_PATCH 0

#define ADDR_ETH 0
#define ADDR_UDP 4

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

const byte mac[] = {0xDE, 0xAD, 0xBE, 0xA8, 0x5C, 0x7B};

EthernetServer webserver(80);
EthernetUDP socket;

bool sdActive;

byte replyBuf[SERIAL_MAX_SIZE > UDP_TX_PACKET_MAX_SIZE ? SERIAL_MAX_SIZE : UDP_TX_PACKET_MAX_SIZE];
byte replySize = 0;

byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

byte serialBuf[SERIAL_MAX_SIZE];
byte serialIndex = 0;

CRGB leds[TOTAL_LEN];

byte statics[5][3];

struct EffectController {
  CRGB (*generator)(byte, byte);
  bool (*effect)(EffectController*);
  unsigned long nextTick;
  byte effectStart;
  byte effectEnd;
  byte data[8];
  byte threadID;
};

typedef bool (*effect)(EffectController*);

typedef CRGB (*generator)(byte, byte);

//This replaces 16 booleans. Use bitwise operations to access
unsigned int effectThreadStates = 0;

EffectController effectThreads[16];

void setup() {

  Serial.begin(115200);

  //FastLED Setup
  LEDS.addLeds<WS2812, DATA_PIN, GRB>(&leds[STRIP_START], STRIP_LEN);
  LEDS.addLeds<WS2811, DATA_PIN, RGB>(&leds[XMAS_START], XMAS_LEN);
  LEDS.setBrightness(50);
  for (byte i = 0; i < TOTAL_LEN; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();

  //Ethernet setup
  Ethernet.begin(mac, IPAddress(EEPROM.read(ADDR_ETH), EEPROM.read(ADDR_ETH + 1), EEPROM.read(ADDR_ETH + 2), EEPROM.read(ADDR_ETH + 3)));
  webserver.begin();
  socket.begin(EEPROM.read(ADDR_UDP) * 256 + EEPROM.read(ADDR_UDP + 1));

  sdActive = SD.begin(4);

}

void loop() {

  runEffectThreads();
  checkSerial();
  checkSocket();
  checkServer();

}
