void initNetwork() {
  webserver = EthernetServer(webserverPort);
  Ethernet.begin(MAC_ADDRESS, IPAddress(ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]));
  udp.begin(udpReceiverPort);
  webserver.begin();
}
