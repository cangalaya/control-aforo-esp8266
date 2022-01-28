void ConnectUDP() {
	Serial.println();
	Serial.println("Starting UDP");

	// in UDP error, block execution
	if (UDP.begin(localPort) != 1) 
	{
		Serial.println("Connection failed");
		while (true) { delay(1000); }
	}

	Serial.println("UDP successful");
	Serial.println("waiting for sync");
	Serial.print("Local port: ");
  	Serial.println(UDP.localPort());

}

void SendUDP_ACK()
{
  //UDP.beginPacket(remoteIP_fijo, remotePort);
	UDP.beginPacket(UDP.remoteIP(), remotePort);
	UDP.write("ACK");
	UDP.endPacket();
}

void SendUDP_Packet(String content)
{
	//UDP.beginPacket(remoteIP_fijo, remotePort);
  UDP.beginPacket(UDP.remoteIP(), remotePort);
	UDP.write(content.c_str());
	UDP.endPacket();
}

// void SendUDP_Packet_Dashboard(String content)
// {
// 	Serial.println("Enviando a Dataloger " + String(remoteIP_dashboard[0])+ "." + String(remoteIP_dashboard[1])+ "." + String(remoteIP_dashboard[2])+ "." + String(remoteIP_dashboard[3])+ " : " + String(remotePortDashboard));
//   UDP.beginPacket(remoteIP_dashboard, remotePortDashboard);
//   UDP.write(content.c_str());
//   UDP.endPacket();
// }

void GetUDP_Packet(bool sendACK = true)
{
	int packetSize = UDP.parsePacket();
	if (packetSize)
	{
		// read the packet into packetBufffer
		UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

		Serial.println();
		Serial.print("Received packet of size ");
		Serial.print(packetSize);
		Serial.print(" from ");
		Serial.print(UDP.remoteIP());
		Serial.print(":");
		Serial.println(UDP.remotePort());
		Serial.print("Payload: ");
		Serial.write((uint8_t*)packetBuffer, (size_t)packetSize);
		Serial.println();
		//ProcessPacket(String(packetBuffer));

		//// send a reply, to the IP address and port that sent us the packet we received
		if(sendACK) SendUDP_ACK();
    ProcessPacket(String(packetBuffer));
	}
	//delay(10);
}
