void setDimmer(byte dimmer, byte value) {

  if (!dimmers[dimmer].enabled) {
    return;
  }

  dimmers[dimmer].value = value;

  if (dimmers[dimmer].bipolar) {
    if (128 <= value) {
      value = 255;
    } else {
      value = 0;
    }
  }

  if (dimmers[dimmer].inverse) {
    value = 255 - value;
  }

  switch (dimmers[dimmer].method) {
    case 0:
      analogWrite(dimmers[dimmer].pin, value);
      break;
  }
  
}

void setRemoteDimmer(byte destAddr, byte dimmer, byte value) {
  Serial.write( ); //TODO
  Serial.write(
}
