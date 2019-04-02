/**
   Sets the output level of the specified dimmer. Accepts dimmer struct, not an index.
*/
void setLevel(Dimmer *dim, byte value) {
  dim->value = value;
  if (dim->inverse) {
    value = 255 - value;
  }
  if (dim->bipolar) {
    if (128 <= value) {
      value = 255;
    } else {
      value = 0;
    }
  }
  switch (dim->method) {
    case 0:
      analogWrite(dim->pin, value);
      break;
  }
}

/**
   Controls the special functions for dimmers. If forceAll is true, it will update the output of all of the enabled dimmers, even if they have no functions applied.
*/
void runDimmers(boolean forceAll) {
  for (byte i = 0; i < 50; i++) {
    if (dimmers[i].enabled) {
      switch (dimmers[i].function) {
        case 0:
          if (forceAll) {
            setLevel(&dimmers[i], dimmers[i].value);
          }
          break;
        case 1:
          //A fading function. data[0] is the final value, data[1] is the step size, and data[2] is the tick skip (runs every data[2] + 1 ticks, so 0 runs on every tick, 1 every other tick, and so on...)
          if (dimmers[i].data[2] == dimmers[i].data[3]) {
            dimmers[i].data[3] = 0;

            if (abs(dimmers[i].data[0] - dimmers[i].value) < dimmers[i].data[1]) {
              setLevel(&dimmers[i], dimmers[i].data[0]);
              dimmers[i].function = 0;
              dimmers[i].data[0] = 0;
              dimmers[i].data[1] = 0;
              dimmers[i].data[2] = 0;
            } else {
              if (dimmers[i].data[0] > dimmers[i].value) {
                setLevel(&dimmers[i], dimmers[i].value + dimmers[i].data[1]);
              } else {
                setLevel(&dimmers[i], dimmers[i].value - dimmers[i].data[1]);
              }
            }

            break;
          } else {
            dimmers[i].data[3]++;
          }
      }
    }
  }
}

/**
   Loads properties from memory and sets necessary pins to outputs
*/
void initDimmers(boolean doPins) {

  if (doPins) {
    for (byte i = 0; i < 50; i++) {
      digitalWrite(dimmers[i].pin, LOW);
      pinMode(dimmers[i].pin, INPUT);
    }
  }

  for (byte i = 0; i < 50; i++) {
    dimmers[i].pin = EEPROM.read(EEPROM.length() - i * 2 - 1);
    byte compProps = EEPROM.read(EEPROM.length() - i * 2 - 2);
    dimmers[i].enabled = bitRead(compProps, 0);
    dimmers[i].bipolar = bitRead(compProps, 1);
    dimmers[i].inverse = bitRead(compProps, 2);
    for (byte i = 0; i < 4; i++) {
      bitWrite(dimmers[i].method, i, bitRead(compProps, i + 4));
    }

  }

  for (byte i = 0; i < 50; i++) {
    if (dimmers[i].enabled) {
      if (pinIsValid(dimmers[i].pin)) {
        pinMode(dimmers[i].pin, OUTPUT);
      } else {
        dimmers[i].enabled = false;
      }
    }
  }

  runDimmers(true);

}

byte getDimmerProperty(byte index, byte prop) {

  switch (prop) {
    case 0:
      return dimmers[index].pin;
    case 1:
      return dimmers[index].enabled;
    case 2:
      return dimmers[index].bipolar;
    case 3:
      return dimmers[index].inverse;
    case 5:
      return dimmers[index].method;
  }

}

void setDimmerProperty(byte index, byte prop, byte value) {
  if (prop == 0) {
    if (!pinIsValid(value)) {
      return;
    }
    EEPROM.update(EEPROM.length() - index * 2 - 1, value);
    initDimmers(true);
  } else if (prop >= 1 && prop <= 4) {
    byte compProps = EEPROM.read(EEPROM.length() - index * 2 - 2);
    bitWrite(compProps, prop - 1, value);
    EEPROM.update(EEPROM.length() - index * 2 - 2, compProps);
    initDimmers(false);
  } else if (prop >= 5 && prop <= 8) {
    byte compProps = EEPROM.read(EEPROM.length() - index * 2 - 2);
    for (byte i = 0; i < 4; i++) {
      bitWrite(compProps, i + 4, bitRead(value, i));
    }
    EEPROM.update(EEPROM.length() - index * 2 - 2, compProps);
    initDimmers(false);
  }
}
