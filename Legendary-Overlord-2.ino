/*

   Legendary-Overlord-2 by Daniel Tierney uses LightingControlMaster

   LightingControlMaster by Daniel Tierney (https://github.com/RandomShrub)

   Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence
   No, the rotary encoder does nothing. Making it do something would be too complicated. If you want it to work, have at it.

*/

#include <EEPROM.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>

#define d_DimmerCount 50

#define d_Pin 0
#define d_Enabled 1
#define d_Bipolar 2
#define d_Inverse 3
#define d_Method 5

#define m_UdpPort 0
#define m_IPAddr 2
#define m_DisplayTimeout 7 //Ideally this would be 6 but cell 6 on my board is cranky and stuck on 255
#define m_TimeFormat 8 //See ↑↑↑
#define m_I2CDisplay 9

#define l_scrollUp 1
#define l_scrollDown 2
#define l_quit 3
#define l_save 4

#define b_DEL '.'
#define b_GO '#'
#define b_CMD '*'
#define b_Up 'A'
#define b_Down 'B'

//Rotary Encoder
const byte encoderPinA = 2;
const byte encoderPinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

//Keypad
const byte noRestriction = 100;
const byte ROWS = 4;
const byte COLS = 4;
const char keys[ROWS][COLS] = {
  {'7', '8', '9', 'A'},
  {'4', '5', '6', 'B'},
  {'1', '2', '3', '/'},
  {'0', '.', '#', '*'}
};
const byte rowPins[ROWS] = {22, 23, 24, 25};
const byte colPins[COLS] = {26, 27, 28, 29};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//Clock
RTC_DS3231 rtc;
const String daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const String daysOfTheWeekShort[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
DateTime now;

//Ethernet
const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x91, 0x39};
byte ip[] = {192, 168, 1, 167};
unsigned int socketPort = 0;
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];
EthernetUDP socket;

//Misc Data
byte dataBuffer[50];
byte replyBuffer[50];
byte replySize = 0;
byte serialIndex = 0;
byte serialLength = 0;

//Display
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
boolean screenHome;
boolean lcdEnabled = true;
byte timeFormat;
byte homeMode;
byte timeoutDuration;
byte timeout;
byte i2cDisplay;

byte arrowUp[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte arrowDown[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

struct Dimmer {
  byte pin;
  byte method;
  byte value; //Use setLevel(Dimmer dim, byte value). This will not update it unless you call runDimmers(true)
  byte function;
  byte data[4];
  boolean enabled;
  boolean bipolar;
  boolean inverse;
  boolean dirty;
};
unsigned long nextDimmerTick;
unsigned long nextSecond;
Dimmer dimmers[d_DimmerCount];

void setup() {

  Serial.begin(115200);

  lcd.begin(16, 2);

  lcd.createChar(0, arrowUp);
  lcd.createChar(1, arrowDown);

  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);
  lcd.print("Init..");

  initDimmers(true);

  Wire.begin();

  rtc.begin();

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), EncoderPinA, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), EncoderPinB, RISING);

  screenHome = true;
  socketPort = 256 * EEPROM.read(m_UdpPort) + EEPROM.read(m_UdpPort + 1);
  timeoutDuration = EEPROM.read(m_DisplayTimeout);
  timeFormat = EEPROM.read(m_TimeFormat);
  i2cDisplay = EEPROM.read(m_I2CDisplay);
  ip[0] = EEPROM.read(m_IPAddr);
  ip[1] = EEPROM.read(m_IPAddr + 1);
  ip[2] = EEPROM.read(m_IPAddr + 2);
  ip[3] = EEPROM.read(m_IPAddr + 3);


  timeout = timeoutDuration;
  now = rtc.now();

  beginNetwork();

  updateHomeScreen();

}

void loop() {

  if (millis() >= nextDimmerTick) {
    nextDimmerTick = millis() + 50;
    runDimmers(false);
  }


  //Run every second
  if (millis() >= nextSecond) {
    if (timeout > 0) {
      timeout--;
      if (timeout == 0) {
        displayOff();
      }
    }

    //Has the time changed?
    if (rtc.now().minute() != now.minute()) {
      now = rtc.now();
      updateHomeScreen();

      
      //Update the 4 x 7seg remote display, if it exists
      if (i2cDisplay != 0) {

        Wire.beginTransmission(i2cDisplay);
        Wire.write(6); //Command byte exclusive to I2C display
        Wire.write(0); //Subcommand for I2C display to recognize it is receiving the time
        Wire.write(timeFormat); //Tell it how to display
        Wire.write(now.hour());
        Wire.write(now.minute());
        Wire.endTransmission(); //No reply is expected
        
      }

      
    } else {
      now = rtc.now();
    }
    
    nextSecond = millis() + 1000;
  }

  keypadKeys();
  checkSocket();

}

void serialEvent() {
  while (Serial.available()) {
    byte newData = Serial.read();
    if (0 == serialLength) {
      serialLength = newData;
    } else {
      dataBuffer[serialIndex++] = newData;
      if (serialIndex >= serialLength) {
        processData(dataBuffer, 'S');
        serialIndex = 0;
        serialLength = 0;
        if (replySize > 0) {
          Serial.write(replySize);
          for (byte i = 0; i < replySize; i++) {
            Serial.write(replyBuffer[i]);
          }
          replySize = 0; //Signals that replyBuffer isn't being used and can be overwritten. No need to clear the whole thing.
        }
      }
    }
  }
}

boolean pinIsValid(byte pin) {
  if (3 >= pin || 10 == pin || 50 <= pin && 52 >= pin || 14 <= pin && 21 >= pin) {
    return false;
  } else {
    return true;
  }
}

/**
   A short pause keeps the dimmer functions running but maintains nothing else.
   Useful for very short pauses where the full loop could cause too long delays, but the dimmers should be kept running.
*/
void shortPause(long duration) {
  duration = millis() + duration;
  while (millis() < duration) {
    if (millis() >= nextDimmerTick) {
      nextDimmerTick = millis() + 50;
      runDimmers(false);
    }
  }
}

/**
   Useful for longer pauses when device functionality needs to be maintained, like when running a UI
*/
void longPause(long duration) {
  duration = millis() + duration;
  while (millis() < duration) {
    if (millis() >= nextDimmerTick) {
      nextDimmerTick = millis() + 50;
      runDimmers(false);
    }

    if (millis() >= nextSecond) {
      updateHomeScreen();
      nextSecond = millis() + 1000;
    }

    checkSocket();
  }
}
