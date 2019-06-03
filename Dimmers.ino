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

void setRemoteLevels(byte address, byte dimCount, byte *dimmerIDs, byte valueCount, byte *values) {

  Wire.beginTransmission(address);

  Wire.write(1);
  
  Wire.write(dimCount);
  for (byte i = 0; i < dimCount; i++) {
    Wire.write(dimmerIDs[i]);
  }

  Wire.write(0);

  Wire.write(valueCount);
  for (byte i = 0; i < valueCount; i++) {
    Wire.write(values[i]);
  }

  Wire.endTransmission();
  
}

void setRemoteLevel(byte address, byte dimmerID, byte value) {
  setRemoteLevels(address, 1, &dimmerID, 1, &value);
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
   Retrieve a property value from a connected I2C device.
   While the data interpreter supports multiple dimmers, I don't yet see any use for this.
*/
byte getRemoteDimmerProperty(byte address, byte dimmerID, byte prop) {

  Wire.beginTransmission(address);
  Wire.write(2);
  Wire.write(3);
  Wire.write(prop);
  Wire.write(1);
  Wire.write(dimmerID);
  Wire.endTransmission();

  delay(10); //Give time to process

  Wire.requestFrom(address, 1); //Only a single byte coming back

  unsigned long stopListening = millis() + 10;

  while (Wire.available() == 0 && millis() < stopListening); //Wait until there is a reply

  return Wire.read(); //It was only 1 byte coming back. No need for anything complicated.


}

/**
   Sets the property of a dimmer and marks it as dirty so it can be written to the memory later.
   bindAll() will save all updated system dimmers[] to the EEPROM. If you are not using these dimmers, you
   must implement your own system to save the properties.

   To avoid having to reference numbers, all properties have constants to help:
   d_Pin, d_Enabled, d_Bipolar, d_Inverse, d_Method

*/
void setDimmerProperty(Dimmer *dim, byte prop, byte value) {

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
      if (value < 16) dim->method = value;
      break;
    default:
      return;
  }

  dim->dirty = true;

}

/**
   Construct a message to a connected I2C device instructing it to change dimmer properties.
   Because dimmers/values are sent in array format, they should be given to this procedure that way.
*/
void setRemoteDimmerProperties(byte address, byte *dimmerIDs, byte idCount, byte prop, byte *values, byte valueCount) {

  Wire.beginTransmission(address);
  Wire.write(1); //Dimmers command

  Wire.write(idCount);
  for (byte i = 0; i < idCount; i++) {
    Wire.write(dimmerIDs[i]);
  }

  Wire.write(3); //Set props subcommand

  Wire.write(prop);

  Wire.write(valueCount);
  for (byte i = 0; i < valueCount; i++) {
    Wire.write(values[i]);
  }

  Wire.endTransmission();

}

void setRemoteDimmerProperty(byte address, byte dimmerID, byte prop, byte value) {

  setRemoteDimmerProperties(address, &dimmerID, 1, prop, &value, 1);

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
