void addLogEntry(String text, byte logLevel) {

  if (SERIAL_HUMAN == serialInterface[0].mode) {
    Serial.println(text);
  }
  if (SERIAL_HUMAN == serialInterface[1].mode) {
    Serial1.println(text);
  }
  if (SERIAL_HUMAN == serialInterface[2].mode) {
    Serial2.println(text);
  }
  if (SERIAL_HUMAN == serialInterface[3].mode) {
    Serial3.println(text);
  }
  
}
