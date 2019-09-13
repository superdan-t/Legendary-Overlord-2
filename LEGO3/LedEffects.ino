void makeRainbow(Preset *preset) {
  for (byte i = 0; i < STRIP_LEN; i++) {
    CRGB tempLed = CHSV(round(i * 1.7F), 255, 255);
    preset->color[i].red = tempLed.red;
    preset->color[i].green = tempLed.green;
    preset->color[i].blue = tempLed.blue;
  }
}

void fillEffect(Preset *fillColor, byte space, byte speed) {
  static byte direction;
  static unsigned long nextEffect;
  static byte nextLed;

  if (direction == 0) {
    FastLED.clear();
    direction = 1;
    nextLed = XMAS_LEN;
  }

  if (millis() >= nextEffect) {
    nextEffect = millis() + (255 - speed) * 10;

    leds[nextLed] = CRGB(fillColor->color[nextLed].red, fillColor->color[nextLed].green, fillColor->color[nextLed].blue);

    if (direction == 1) {
      nextLed += space;
      if (nextLed >= STRIP_LEN) {
        nextLed = nextLed - space + 1;
        direction = 2;
      }
    } else {
      nextLed -= space;
      if (nextLed < XMAS_LEN) {
        nextLed = nextLed + space - 1;
        direction = 1;
      }
    }

  }

}
