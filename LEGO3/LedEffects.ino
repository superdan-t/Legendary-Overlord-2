effect getEffectWithID(byte id) {
  switch (id) {
    case 1:
      return shiftForever;
    case 2:
      return mySampleEffect;
    case 3:
      return strobe;
    default:
      return shiftForever;
  }
}

//Static effect, no thread
void shiftDown(byte lowerBound, byte upperBound) {
  CRGB swap = leds[lowerBound];
  for (byte i = lowerBound; i < upperBound; i++) {
    leds[i] = leds[i + 1];
  }
  leds[upperBound] = swap;
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
    ec->nextTick = millis() + ec->data[0];
    if (ec->data[1]) {
      shiftDown(0, TOTAL_LEN - 1);
    } else {
      shiftUp(0, TOTAL_LEN - 1);
    }
    FastLED.show();
  }
  return false;
}

bool strobe(EffectController *ec) {
  if (millis() >= ec->nextTick) {
    if (ec->data[2]) {
      ec->nextTick = millis() + ec->data[0];
      for (byte i = ec->effectStart; i < ec->effectEnd; i++) {
        leds[i] = CRGB(0, 0, 0);
      }
      ec->data[2] = false;
    } else {
      ec->nextTick = millis() + ec->data[1];
      for (byte i = ec->effectStart; i < ec->effectEnd; i++) {
        leds[i] = ec->generator(i - ec->effectStart, ec->effectEnd - ec->effectStart);
      }
      ec->data[2] = true;
    }
    FastLED.show();
  }
  return false;
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
