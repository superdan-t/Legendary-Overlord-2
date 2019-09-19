CRGB (*getGeneratorWithID(byte id)) (byte, byte) {
  switch (id) {
    case 1:
      return getSpectrum;
    case 2:
      return getPatriotic;
    default:
      return getCurrentColors;
  }
}

//Uses length to find the RGB value of a certain LED that falls within that spectrum
CRGB getSpectrum(byte position, byte length) {
  static float stepSize;
  static byte lastLength;
  if (lastLength != length) {
    stepSize = 255.0F / length;
    lastLength = length;
  }
  CRGB tempLED = CHSV(position * stepSize, 255, 255);
  return tempLED;
}

CRGB getPatriotic(byte position, byte length) {
  static float thirdConstant;
  static float lastLength;
  if (lastLength != length) {
    thirdConstant = 255.0F / (length / 3);
    lastLength = length;
  }

  if (position < length / 3) {
    byte gb = thirdConstant * position;
    Serial.print(255);
    Serial.print("\t");
    Serial.print(gb);
    Serial.print("\t");
    Serial.println(gb);
    return CRGB(255, gb, gb);

  } else if (position > length / 3 * 2) {
    byte rg = thirdConstant * (length - position);
    Serial.print(rg);
    Serial.print("\t");
    Serial.print(rg);
    Serial.print("\t");
    Serial.println(255);
    return CRGB(rg, rg, 255);
  } else {
    Serial.print(255);
    Serial.print("\t");
    Serial.print(255);
    Serial.print("\t");
    Serial.println(255);
    return CRGB(255, 255, 255);
  }


}

CRGB getCurrentColors(byte position, byte length) {
  return leds[position];
}
