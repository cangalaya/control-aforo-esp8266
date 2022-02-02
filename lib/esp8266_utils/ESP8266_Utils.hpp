void ConnectWiFi_STA(bool useStaticIP = false)
{
   Serial.println("");
   WiFi.mode(WIFI_STA);
   // if ((EEPROM.read(0) == 255 || EEPROM.read(0) == 0)  || (EEPROM.read(50) == 255 ||  EEPROM.read(50)==0))    // se inicia del sistema y no está grabado nada en memoria
   // {
      Serial.println("-->>> SSID and PASSWORD defauld");
      WiFi.begin(ssid, password);
   // } else {
   //    WiFi.begin(readStringFromEEPROM(0), readStringFromEEPROM(50));
   // }
   if(useStaticIP) WiFi.config(ip, gateway, subnet);
    int counter_wifi = 0;
   while (WiFi.status() != WL_CONNECTED) 
   { 
     delay(500);  
     Serial.print('.');
     counter_wifi++;
     if (counter_wifi == 100){
      Serial.print("Reseting ESP");
      ESP.restart();
     }
   }
 
   Serial.println("");
   Serial.print("Iniciado STA:\t");
   Serial.println(readStringFromEEPROM(0));
   Serial.print("IP address:\t");
   Serial.println(WiFi.localIP());
}

void ConnectWiFi_AP(bool useStaticIP = false)
{ 
   Serial.println("");
   WiFi.mode(WIFI_AP);
   while(!WiFi.softAP(ssid, password))
   {
     Serial.println(".");
     delay(100);
   }
   if(useStaticIP) WiFi.softAPConfig(ip, gateway, subnet);

   Serial.println("");
   Serial.print("Iniciado AP:\t");
   Serial.println(ssid);
   Serial.print("IP address:\t");
   Serial.println(WiFi.softAPIP());
}
