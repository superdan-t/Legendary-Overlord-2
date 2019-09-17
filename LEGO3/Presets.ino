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
