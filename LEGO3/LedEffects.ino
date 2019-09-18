//Static effect, no thread
void shiftDown(byte lowerBound, byte upperBound) {
  CRGB swap = leds[lowerBound];
  for (byte i = lowerBound; i < upperBound; i++) {
    leds[i] = leds[i + 1];
//    Serial.print(i);
//    Serial.print(" gets the value of ");
//    Serial.println(i + 1);
  }
  leds[upperBound] = swap;
//  Serial.print(upperBound);
//  Serial.print(" gets the value of ");
//  Serial.println(lowerBound);
}

void shiftUp(byte lowerBound, byte upperBound) {
  CRGB swap = leds[upperBound];
  for (byte i = upperBound; i > lowerBound; i--) {
    leds[i] = leds[i - 1];
  }
  leds[lowerBound] = swap;
}

//There's no going back. Unless you kill this thread that is
bool shiftForever(EffectController *ec) {
  if (millis() >= ec->nextTick) {
    ec->nextTick = millis() + 100;
    shiftDown(0, TOTAL_LEN - 1);
    FastLED.show();
  }
}


//Works as a thread
bool mySampleEffect(EffectController *ec) {
  if (millis() >= ec->nextTick) {
    ec->nextTick = millis() + 1000;
    Serial.println("Sample effect with ID " + String(ec->threadID) + " has run " + String(ec->data[0]) + " times.");
    unsigned long startTime = millis();
    ec->generator(0, TOTAL_LEN);
    for (byte i = 0; i < TOTAL_LEN; i++) {
      int g = i + ec->data[0] * 50;
      if (g >= TOTAL_LEN) {
        g = ec->data[0] * 50 - TOTAL_LEN + i;
      }
      leds[i] = ec->generator(g, TOTAL_LEN);
    }
    unsigned long endTime = millis();
    Serial.print("That took ");
    Serial.print(endTime - startTime);
    Serial.println(" ms to run.");
    FastLED.show();
    ec->data[0]++;
    if (ec->data[0] == 5) {
      Serial.println("Deactivating. This shouldn't print again.");
      return true;
    }
  }
  return false;
}

//bool fillEffect(Preset *fillColor, byte space, byte speed) {
//  static byte direction;
//  static unsigned long nextEffect;
//  static byte nextLed;
//  static byte cycler;
//
//  if (direction == 0) {
//    FastLED.clear();
//    direction = 1;
//    nextLed = 0;
//  }
//
//  if (millis() >= nextEffect) {
//    nextEffect = millis() + (256 - speed) * 10;
//    Serial.println("M: " + String(millis()));
//    Serial.println("N: " + String(nextEffect));
//
//    leds[nextLed] = CRGB(fillColor->color[nextLed].red, fillColor->color[nextLed].green, fillColor->color[nextLed].blue);
//
//    FastLED.show();
//
//    cycler++;
//
//    if (cycler >= TOTAL_LEN) {
//      cycler = 0;
//      return 0;
//    }
//
//    if (direction == 1) {
//      nextLed += space;
//      if (nextLed >= TOTAL_LEN) {
//        nextLed = nextLed - space + 1;
//        direction = 2;
//      }
//    } else {
//      nextLed -= space;
//      if (nextLed >= TOTAL_LEN) {
//        nextLed = nextLed + space + 1;
//        direction = 1;
//      }
//    }
//
//  }
//
//  return 1;
//
//}
