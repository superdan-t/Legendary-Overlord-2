void beginNetwork() {
  Ethernet.begin(mac, IPAddress(ip[0], ip[1], ip[2], ip[3]));
  socket.begin(socketPort);
}

void checkSocket() {

  if (socket.parsePacket() == 0) {
    return;
  }

  socket.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

  processData(packetBuffer, 'I');


  socket.beginPacket(socket.remoteIP(), socket.remotePort());

  if (replySize != 0) {
    socket.write(replySize);
    socket.write(replyBuffer, replySize);
    replySize = 0;
  } else {
    socket.write(1);
    socket.write(6);
  }

  socket.endPacket();


}
