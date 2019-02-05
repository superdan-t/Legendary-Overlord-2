void initSerial(byte port) {
  if (serialInterface[port].mode != SERIAL_DISABLED) {
    switch (port) {
      case 0:
        Serial.end();
        Serial.begin(serialSpeed(serialInterface[port].mode));
        break;
      case 1:
        Serial1.end();
        Serial1.begin(serialSpeed(serialInterface[port].mode));
        break;
      case 2:
        Serial2.end();
        Serial2.begin(serialSpeed(serialInterface[port].mode));
        break;
      case 3:
        Serial3.end();
        Serial3.begin(serialSpeed(serialInterface[port].mode));
        break;
      default:
        errorLevel = ERROR_OUT_OF_RANGE;
        break;
    }
  }
}

long serialSpeed(byte protocol) {
  switch (protocol) {
    case SERIAL_SDSCP:
      return 115200;
    case SERIAL_SDSCP2:
      return 250000;
    case SERIAL_MIDI:
      return 31250;
    case SERIAL_HUMAN:
      return 115200;
    default:
      errorLevel = ERROR_OUT_OF_RANGE;
      return 0;
  }
}

void initSerial() {
  initSerial(0);
  initSerial(1);
  initSerial(2);
  initSerial(3);
}

void serialEvent() {
  
}
