void initNetwork() {
  //webserver = EthernetServer(webserverPort);
  Ethernet.begin(MAC_ADDRESS, IPAddress(ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]));
  udp.begin(udpReceiverPort);
  //webserver.begin();
}

void udpServer() {
  int packetSize = udp.parsePacket();
  if (0 < packetSize) {
//    udp.read(dataBuffer, UDP_TX_PACKET_MAX_SIZE);

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(replyBuffer, replySize);
    udp.endPacket();
    
  }
}
