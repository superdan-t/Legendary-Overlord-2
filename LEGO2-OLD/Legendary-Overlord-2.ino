//Created 2/1/2019

#include <EEPROM.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

//Operational Constants

#define ERROR_OUT_OF_RANGE 1

#define PERM_SERIAL 1
#define PERM_UDP 2
#define PERM_SCRIPT 3
#define PERM_HUMAN 4

#define EEPROM_VARIABLE_MAX 6
#define MEM_WEB_PORT 0
#define MEM_UDP_PORT 2
#define MEM_IP 4
#define MEM_SYSNAME 9
#define MEM_SERIAL_MODE 10

#define LOG_URGENT 0
#define LOG_INFO 1

#define SERIAL_DISABLED 0
#define SERIAL_SDSCP 1
#define SERIAL_SDSCP2 2
#define SERIAL_MIDI 3
#define SERIAL_HUMAN 4

byte errorLevel = 0;
byte systemName = 0;

byte replyBuffer[UDP_TX_PACKET_MAX_SIZE >= 50 ? UDP_TX_PACKET_MAX_SIZE : 50];
byte replySize;

//Clock
const String daysOfTheWeekShort[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
DateTime now;
RTC_DS3231 rtc;

//Internet Variables

const byte MAC_ADDRESS[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x91, 0x39
};
unsigned int webserverPort = 80;
unsigned int udpReceiverPort = 1154;
byte udpBuffer[UDP_TX_PACKET_MAX_SIZE];
byte ipAddress[] = {192, 168, 1, 150};
EthernetUDP udp;
EthernetServer webserver(webserverPort);

//Serial Variables

struct SerialInterface {
  byte mode = 0;
  boolean busy = 0;
  byte data[50];
};

SerialInterface serialInterface[4];

//LCD Variables
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Dimmers
struct Dimmer {
  byte pin;
  byte method;
  byte value;
  byte function;
  byte data[3];
  boolean enabled;
  boolean bipolar;
  boolean inverse;
};

Dimmer dimmers[64];

void setup() {

  rtc.begin();

  lcd.begin(16, 2);
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  loadAllVariables();
  initSerial();
  initNetwork();

}

void loop() {
  // put your main code here, to run repeatedly:

}
