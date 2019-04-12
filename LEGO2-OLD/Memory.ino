void loadAllVariables() {
  webserverPort = EEPROM.read(MEM_WEB_PORT) * 256 + EEPROM.read(MEM_WEB_PORT + 1);
  udpReceiverPort =EEPROM.read(MEM_UDP_PORT) * 256 + EEPROM.read(MEM_UDP_PORT + 1);
  ipAddress[0] = EEPROM.read(MEM_IP);
  ipAddress[1] = EEPROM.read(MEM_IP + 1);
  ipAddress[2] = EEPROM.read(MEM_IP + 2);
  ipAddress[3] = EEPROM.read(MEM_IP + 4);
  systemName = EEPROM.read(MEM_SYSNAME);
  serialInterface[0].mode = EEPROM.read(MEM_SERIAL_MODE);
  serialInterface[1].mode = EEPROM.read(MEM_SERIAL_MODE + 1);
  serialInterface[2].mode = EEPROM.read(MEM_SERIAL_MODE + 2);
  serialInterface[3].mode = EEPROM.read(MEM_SERIAL_MODE + 3);
}

void restoreDefaults() {
  for (byte i = 12; i < EEPROM.length(); i++) {
    EEPROM.update(i, 0);
  }
  
  EEPROM.update(MEM_WEB_PORT, 0);
  EEPROM.update(MEM_WEB_PORT + 1, 80);
  EEPROM.update(MEM_UDP_PORT, 4);
  EEPROM.update(MEM_UDP_PORT + 1, 130);
  EEPROM.update(MEM_IP, 192);
  EEPROM.update(MEM_IP + 1, 168);
  EEPROM.update(MEM_IP + 2, 1);
  EEPROM.update(MEM_IP + 4, 150);
  EEPROM.update(MEM_SERIAL_MODE, SERIAL_HUMAN);
  EEPROM.update(MEM_SERIAL_MODE + 1, SERIAL_SDSCP);
  
}
