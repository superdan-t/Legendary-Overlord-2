void interpretData(byte dataBuffer[], byte permissionLevel) {

  dataReplySize = 0;

  if (0 == dataBuffer[0]) {
    
    replyAck(true);
    
  } else if (1 == dataBuffer[0]) {
    
    for (byte i = 2; i < 2 + dataBuffer[1]; i++) {
      
      if (dataBuffer[2 + dataBuffer[1]] >= dataBuffer[1]) {
        
      } else if (0 != dataBuffer[2 + dataBuffer[1]]) {
        
      } else {
        
      }
      
    }
    
  }
}

void replyAck(boolean type) {
  dataReplySize = 1;
  if (type) {
    replyBuffer[0] = 6;
  } else {
    replyBuffer[0] = 21;
  }
}
