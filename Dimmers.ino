/**
   Doesn't do anything. Just prints the values so you can see them
*/
void setLevelTest(byte dimmerIndex, byte value) {
  Serial.println("Set dimmer " + (String)dimmerIndex + " to level " + (String)value);
}

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
  for (byte i = 0; i < d_DimmerCount; i++) {
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
void initDimmers(boolean loadMem) {

  for (byte i = 0; i < d_DimmerCount; i++) {
    digitalWrite(dimmers[i].pin, LOW);
    pinMode(dimmers[i].pin, INPUT);
  }
  
  if (loadMem) {
    for (byte i = 0; i < d_DimmerCount; i++) {
      dimmers[i].pin = EEPROM.read(EEPROM.length() - i * 2 - 1);
      byte compProps = EEPROM.read(EEPROM.length() - i * 2 - 2);
      dimmers[i].enabled = bitRead(compProps, 0);
      dimmers[i].bipolar = bitRead(compProps, 1);
      dimmers[i].inverse = bitRead(compProps, 2);
      for (byte i = 0; i < 4; i++) {
        bitWrite(dimmers[i].method, i, bitRead(compProps, i + 4));
      }

    }
  }

  for (byte i = 0; i < d_DimmerCount; i++) {
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

byte getDimmerProperty(Dimmer *dim, byte prop) {
  switch (prop) {
    case 0:
      return dim->pin;
    case 1:
      return dim->enabled;
    case 2:
      return dim->bipolar;
    case 3:
      return dim->inverse;
    case 5:
      return dim->method;
  }
}

/**
   Sets the property of a dimmer and marks it as dirty so it can be written to the memory later.
   bindAll() will save all updated system dimmers[] to the EEPROM. If you are not using these dimmers, you
   must implement your own system to save the properties.

   To avoid having to reference numbers, all properties have constants to help:
   d_Pin, d_Enabled, d_Bipolar, d_Inverse, d_Method

*/
void setDimmerProperty(Dimmer *dim, byte prop, byte value) {

  Serial.println("Property " + (String)prop + " was set to " + (String)value);

  switch (prop) {
    case 0:
      if (pinIsValid(value)) {
        dim->pin = value;
        initDimmers(false);
      }
      break;
    case 1:
      dim->enabled = value;
      break;
    case 2:
      dim->bipolar = value;
      break;
    case 3:
      dim->inverse = value;
      break;
    case 5:
      dim->method = value;
      break;
    default:
      return;
  }

  dim->dirty = true;

}

/**
   Saves any updated system dimmers to the EEPROM
*/
void bindAll() {
  byte compProps;
  for (int i = 0; i < d_DimmerCount; i++) {
    if (dimmers[i].dirty) {
      //Pin (byte), method (nyble), and 3 bools are stored. 3 bools and the nyble can be shoved into 1 byte.
      compProps = 0;
      bitWrite(compProps, 0, dimmers[i].enabled);
      bitWrite(compProps, 1, dimmers[i].bipolar);
      bitWrite(compProps, 2, dimmers[i].inverse);
      bitWrite(compProps, 4, bitRead(dimmers[i].method, 0));
      bitWrite(compProps, 5, bitRead(dimmers[i].method, 1));
      bitWrite(compProps, 6, bitRead(dimmers[i].method, 2));
      bitWrite(compProps, 7, bitRead(dimmers[i].method, 3));
      if (EEPROM.read(EEPROM.length() - i * 2 - 1) != dimmers[i].pin) {
        EEPROM.update(EEPROM.length() - i * 2 - 1, dimmers[i].pin);
      }
      if (compProps != EEPROM.read(EEPROM.length() - i * 2 - 2)) {
        EEPROM.update(EEPROM.length() - i * 2 - 2, compProps);
      }

    }
  }
}
/*
   Obsolete and included for reference
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
  }*/
