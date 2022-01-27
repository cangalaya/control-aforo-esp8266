/* ------------- CONTROL DE AFORO - 2 PUERTAS COMUNICADAS -----------------
 *  Desarrollado por: Ing. Alonso Cangalaya (cangalaya.2007@gmail.com)
 *  Funciones:
 *    - Realiza conteo ABCD (sensores S1 y S2 conectados a este ESP8266)
 *    - Comunicación WiFi con protocolo UDP -> este dispositivo es el MAIN GATE
 *    - Comunicación con el Arduino Nano para mostrar datos en el display
 *    - Comunicación con el Dashboard (LabView) por protocolo UDP
 */

///////////// DEPENDENCIAS ///////////
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h> // Protocolo UDP (transferencia de datos rapida, pero no segura)
#include <Separador.h>           // liberia para separa string (se usa para separar valores en la trama de datos de comunicación con el UDP)
#include <Firebase_ESP_Client.h> //#include <Firebase_ESP_Client.h>
#include <EEPROM.h>              // update de memorias fash


///// VARIABLES PARA EL RESET DE AFORO ////
int minuto = 0;
int minuto_anterior = 0;
//int counter_minutos_inactividad = 0;

//----------------------------------------DMD Configuration (P10 Panel)
//#define DISPLAYS_WIDE 3 //--> Panel Columns         <<<<<<<<<<<<<<< D I S P L A Y   C O L U M N A S <<<<<<<<<<<
//#define DISPLAYS_HIGH 1 //--> Panel Rows
// DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);          //Creación de Instancia llamada Disp -> Number of Panels P10 used (Column, Row)
//----------------------------------------
Separador s; // instancia para separar string
//----------------------------------------
// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// creación de JSON
FirebaseJson jsonConfigWifi;
FirebaseJson jsonConfigUdp;
FirebaseJson jsonConfigTarjet;
FirebaseJson jsonData;

//////////// variables para reseteo por inactividad /////////////
unsigned long time_millis = 0;
unsigned long before_time_millis = 0;
unsigned long counter_millis = 0;

#include "config.h" // Configurar datos de la red
#include "UDP.hpp"
#include "ESP8266_Utils.hpp"
#include "ESP8266_Utils_UDP.hpp"

//////////// variables FIREBASE   ////////////////

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 2. Define the API Key */
#define API_KEY "AIzaSyACwNdd8nTeRCLSr6tFyZLQ-jquw8ljKa8"

/* 3. Define the RTDB URL */
#define DATABASE_URL "prueba-2-e4543-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "prueba-2-e4543"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "comedor1@tasa-callao.com"
#define USER_PASSWORD "comedor1"




//=================================================================================================================================================
//============================================================== R U T I N E S ====================================================================
//=================================================================================================================================================
// Subrutina para el sensado de personas al salir o entrar || incluye filtro ABDC  || Modifica los valores de BDatos
String mensaje = "";
void listenPeopleCounting()
{
  if (Serial.available())
  {
    mensaje = Serial.readStringUntil('\n');
    if (mensaje.startsWith("/", 0)) // si es que el string empieza con / aforo y total recibidos por el esp
    {
      //aforo = s.separa(mensaje, '/', 1).toInt();
      BDatos.total = s.separa(mensaje, '/', 2).toInt();
      Serial.println("total actualizado ->"+ String(BDatos.total));
    }

  }
  if (last_value != BDatos.total)
  {
    if (last_value < BDatos.total)
    {
      // si la persona entro
      //     {
      Serial.print("entro | "); // entro
      //BDatos.total++;           // seteamos los valores locales
      BDatos.ingresos++;
      if (BDatos.total > BDatos.aforo)
        BDatos.excesos++;
      Serial.println("Aforo: " + String(BDatos.aforo) + "  Total: " + String(BDatos.total)); // entro
      // SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);    // el elemento separador es el |
      flag_send_data = 1; // enviamos la data actualizada
      // actualizar_pantalla();                                                                    // actualizamos pantalla con nuevos valores
      //counter_minutos_inactividad = 0; // reset a minutos de inactividad
      counter_millis = 0;              // reseteamos el counter_millis para anular el reseteo por inacctividad
    }
    else
    {
      Serial.print("salio | ");  // salio
      if (BDatos.total - 1 >= 0) // protección para que no adopte valores negativos
      {
        BDatos.total--;
        BDatos.egresos++;
        // SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
        flag_send_data = 1; // enviamos la data actualizada
        // actualizar_pantalla();
        //counter_minutos_inactividad = 0;                                                     // reset a minutos de inactividad
        counter_millis = 0;                                                                  // reseteamos el counter_millis para anular el reseteo por inacctividad
      }                                                                                      // actualizamos pantalla con nuevos valores
      Serial.println("Aforo: " + String(BDatos.aforo) + "  Total: " + String(BDatos.total)); // entro
    }
    last_value = BDatos.total;
  }

}

//===============================================================================================================================================
//===============================================================================================================================================
// CLOCK ROUTINES

// NTP Servers:
static const char ntpServerName[] = "south-america.pool.ntp.org";

const int timeZone = -5; // Perú

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
// byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

// REINICIO DE CUENTA
bool reinicioCuenta = true;
void reinicio_de_cuenta()
{
  if (hour() == hora_reinicio && minute() == minuto_reinicio && second() == 0 && reinicioCuenta)
  {
    Serial.print("Reinicio de Cuenta ------ día nuevo\n");
    BDatos.total = 0;
    BDatos.ingresos = 0;
    BDatos.egresos = 0;
    BDatos.excesos = 0;
    reinicioCuenta = false;
  }
  if (minute() != minuto_reinicio)
    reinicioCuenta = true;
}
unsigned long int PrevMillis = 0;
void envioDataRealtime()
{
  if (millis() < 2000)
    PrevMillis = 0; // protección frente a desborde de millis
  if ((Firebase.ready() && (millis() - PrevMillis > (readStringFromEEPROM(240).toInt() * 1000))) && (counter_millis > 5000))
  {
    Serial.println(">>> Enviando datos RealTime >>>");
    PrevMillis = millis();
    //   int integer = 0;
    //   Firebase.RTDB.getInt(&fbdo, "/tasa/callao/bano-varones/config/aforo") ? integer = fbdo.to<int>() : Serial.print(fbdo.errorReason().c_str());
    //   Serial.println(integer);
    //  FirebaseJson jVal;
    //  Serial.printf("Get json ref... %s\n", Firebase.RTDB.getJSON(&fbdo, "/tasa/callao/bano-varones/config", &jVal) ? jVal.raw() : fbdo.errorReason().c_str());
    jsonData.clear();
    jsonData.set("total", BDatos.total);
    jsonData.set("ingresos", BDatos.ingresos);
    jsonData.set("egresos", BDatos.egresos);
    jsonData.set("excesos", BDatos.excesos);
    Serial.printf("Update json... %s\n\n", Firebase.RTDB.updateNodeAsync(&fbdo, path_data + fbdo.pushName(), &jsonData) ? "ok" : fbdo.errorReason().c_str());

  }
}

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (UDP.parsePacket() > 0)
    ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = UDP.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
      UDP.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

//===============================================================================================================================================
//===============================================================================================================================================

void setup()
{
  delay(1000);
  Serial.begin(9600);
  // ---------------- EEPROM (FLASH MEMORY) ----------
  EEPROM.begin(260);

  //-------------- WIFI -----------------
  ConnectWiFi_STA();

  // -------------- FIREBASE -------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  // Optional, use classic HTTP GET and POST requests.
  // This option allows get and delete functions (PUT and DELETE HTTP requests) works for
  // device connected behind the Firewall that allows only GET and POST requests.
  Firebase.RTDB.enableClassicRequest(&fbdo, true);

  /// ------ EPROM INIT ----
  // condicional puesto para validar que los datos esten escritos en firebase
  if (Firebase.ready())
  {
    if (String("null") == String(Firebase.RTDB.getString(&fbdo, path_config + "/" + "wifi" + "/" + "ssid" + "/") ? fbdo.to<const char *>() : fbdo.errorReason().c_str()))
    {
      Serial.println(">>> Estableciendo datos por defecto en Firebase");
      jsonConfigDataSetFirstStart(); // si  no estan escritos primero crea los datos en la nube
      Serial.println("<<< Grabando datos en memoria");
      setCofigEprom();     // luego los lee y guarda en eeprom
    }
    else
    {
      Serial.println("<<< Lectura de Data en Firebase y guardado en la EEPROM");
      setCofigEprom();     // si ya existen los datos, primero lee y guarda el la eeprom
      jsonConfigDataSet(); // escribe los datos los nuevos datos en la nuebe
    }
  }
  else
  {
    setCofigEprom(); // si firebase no se establece solo se configura
  }

  /// ---- SETEO DE CONFIGURACIÓN INICIAL  ----
  BDatos.aforo = readStringFromEEPROM(190).toInt();
  horas_inactividad_max = readStringFromEEPROM(230).toFloat();

  localPort = readStringFromEEPROM(150).toInt();
  remotePort = readStringFromEEPROM(160).toInt();

  /// ---- UDP INIT -----
  ConnectUDP();
  //--------------- CLOCK NTP SERVER ----------------
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

int inicio_r = 0;
unsigned int refresh = 0; // variable para enviar datos cada 2 segundos, sistema de defensa frente a perdida de datos udp

void loop()
{
  reinicio_de_cuenta(); // reinicio a 0 de ingresos, egresos y total cuando ocurre un nuevo día.
  envioDataRealtime();  // envio de data realtime
  actualizarConfigFlash();
  GetUDP_Packet(false); // esperar a que llegen paquetes

  listenPeopleCounting();

  time_millis = millis();
  if (time_millis < 500)
  {                         // cuando millis desb orde despues de 50d aprox.
    before_time_millis = 0; // volvemos el tiempo_aux a 0;
  }

  if (before_time_millis != time_millis)
  {
    before_time_millis = time_millis;
    counter_millis++;
    // Serial.println("Segundos transcurridos: " + String(counter_millis));
  }
  if (counter_millis >= (3600000 * horas_inactividad_max))
  { //(60000*horas_inactividad_max)    60000 = 1min    1hora(60min)= 3600000
    BDatos.total = 0;
    counter_millis = 0;
    Serial.println("Reseteo del total por tiempo de innactividad");
  }

  if (inicio_r == 0) // condicional puesta el setear datos cuando cualquiera de las 2 puertas se apaga repentinamente
  {
    inicio_r = 1;
    // actualizar_pantalla();
    mandar_data_display(); //<<<<<<<<
    // BDatos.estado_inicial = 1;                      //salimos del estado incial
    SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    BDatos.estado_inicial = 1; // salimos del estado incial
  }

  if (flag_send_data == 1) // condicional para enviar datos iniciales
  {
    flag_send_data = 0;
    SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    ////////////////////////////// ENVIO DE DATOS AL DASHBOARD //////////////////////////////
    SendUDP_Packet_Dashboard(String() + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    mandar_data_display(); //<<<<<<<<
    Serial.println("Datos Enviados");
  }

  // ESCRIBIR EN DATABASE FIRESTORE
  // if ((Firebase.ready() && (millis() - PrevMillis > 5000)) || PrevMillis == 0)
  // {
  //   PrevMillis = millis();
  //   String timeStamp = String(String(year()) + "-" + String(month()) + "-" + String(day()) + "--" + String(hour()) + "-" + String(minute()));
  //   Serial.println(timeStamp);

  //   documentPath = "tasa/pisco-sur/comedor-secundario/" + timeStamp;

  //   // If the document path contains space e.g. "a b c/d e f"
  //   // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

  //   content.clear();
  //   content.set("fields/aforo/integerValue", String(BDatos.aforo).c_str());
  //   content.set("fields/total/integerValue", String(BDatos.total).c_str());
  //   content.set("fields/egresos/integerValue", String(BDatos.egresos).c_str());
  //   content.set("fields/ingresos/integerValue", String(BDatos.ingresos).c_str());
  //   //content.set("fields/excesos/integerValue", String(random(1, 20)).c_str());

  //   Serial.print("Create a document... ");

  //   if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
  //     Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  //   else
  //     Serial.println(fbdo.errorReason());
  // }

  //// OBTENCION DE VALORES INT DE REALTIME FIREBASE EJECUTAR EN INTERVALOS DE HORAS
  // if ((Firebase.ready() && (millis() - PrevMillis > 5000) || PrevMillis == 0))
  // {
  //   PrevMillis = millis();
  //   int integer = 0;
  //   Firebase.RTDB.getInt(&fbdo, "/tasa/callao/bano-varones/config/aforo") ? integer = fbdo.to<int>() : Serial.print(fbdo.errorReason().c_str());
  //   Serial.println(integer);
  // }
  
  
  refresh++;
  if (refresh == 65000)
  {
    refresh = 0;
    flag_send_data = 1;
  }
}