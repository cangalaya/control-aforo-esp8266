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
//#include <DMDESP.h>               // librería para el display
#include <Separador.h>           // liberia para separa string (se usa para separar valores en la trama de datos de comunicación con el UDP)
#include <Firebase_ESP_Client.h> //#include <Firebase_ESP_Client.h>
#include <EEPROM.h>              // update de memorias fash
#include <EEPROM.h>
//#include <fonts/SystemFont5x7.h>  //liberias para la tipología de fuente a mostrar en pantalla
//#include <fonts/Droid_Sans_12.h>
//#include <fonts/Arial_Black_16.h>
//#include <fonts/Arial14.h>

//--------- Fuentes para el Display ------------------

// const uint8_t *FONT1 = Arial14;
// const uint8_t *FONT2 = Arial_Black_16;
// const uint8_t *FONT3 = SystemFont5x7;
// const uint8_t *FONT4 = Droid_Sans_12;
// const uint8_t *FONT = FONT2;

///// VARIABLES PARA EL RESET DE AFORO ////
int minuto = 0;
int minuto_anterior = 0;
int counter_minutos_inactividad = 0;

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




////// SETEO INICIAL DE DATOS //////////7
void jsonConfigDataSet()
{

  // SETEO DE CONFIGURACIÓN DE TARJET
  jsonConfigTarjet.add("abcd", readStringFromEEPROM(200));
  jsonConfigTarjet.add("aforo", readStringFromEEPROM(190).toInt());
  jsonConfigTarjet.add("count-delay-milisegundos", readStringFromEEPROM(210).toInt());
  jsonConfigTarjet.add("estado", readStringFromEEPROM(220));
  jsonConfigTarjet.add("inactivity-hours-reset", readStringFromEEPROM(230).toInt());
  jsonConfigTarjet.add("set-data-realtime-segundos", readStringFromEEPROM(240).toInt());

  // SETEO DE CONFIGURACIÓN WIFI
  jsonConfigWifi.add("hostname", readStringFromEEPROM(100));
  jsonConfigWifi.add("ssid", readStringFromEEPROM(0));
  jsonConfigWifi.add("password", readStringFromEEPROM(50));

  // SETEO DE CONFIGURACIÓN UDP
  jsonConfigUdp.add("master-port", readStringFromEEPROM(150).toInt());
  jsonConfigUdp.add("second-port", readStringFromEEPROM(160).toInt());
  jsonConfigUdp.add("datalogger-ip", readStringFromEEPROM(180).toInt());
  jsonConfigUdp.add("datalogger-port", readStringFromEEPROM(170).toInt());

  // SETEO DE CONFIGURACIÓN DATA - REALTIME
  jsonData.add("egresos", BDatos.egresos);
  jsonData.add("excesos", BDatos.excesos);
  jsonData.add("ingresos", BDatos.ingresos);
  jsonData.add("total", BDatos.total);

  if (Firebase.RTDB.setJSONAsync(&fbdo, path_config + "/wifi", &jsonConfigWifi))
  {
    Serial.println("Datos WIFI Firebase Realtime establecidos !!");
  }
  else
  {
    Serial.println("*** Datos WIFI no establecidos en Firebase");
  }

  if (Firebase.RTDB.setJSONAsync(&fbdo, path_config + "/tarjet", &jsonConfigTarjet))
  {
    Serial.println("Datos TARJET Firebase Realtime establecidos !!");
  }
  else
  {
    Serial.println("*** Datos TARJET no establecidos en Firebase");
  }

  if (Firebase.RTDB.setJSONAsync(&fbdo, path_config + "/udp", &jsonConfigUdp))
  {
    Serial.println("Datos UDP Firebase Realtime establecidos !!");
  }
  else
  {
    Serial.println("*** Datos UDP no establecidos en Firebase");
  }

  Serial.printf("Set jsonData... %s\n", Firebase.RTDB.setJSONAsync(&fbdo, path_data, &jsonData) ? "ok" : fbdo.errorReason().c_str());
}

//////////// variables para reseteo por inactividad /////////////
unsigned long time_millis = 0;
unsigned long before_time_millis = 0;
unsigned long counter_millis = 0;

//========== variables para la subrutina - censusPeople() ===========

byte Sensor_1 = LOW;
byte Sensor_2 = LOW;
#define Sensor_1 4
#define Sensor_2 5
#define PtoBuzzer 2 /////////<<<<<<<<< SALIDA -> BUZZER

int cambio_pantalla = 0;

int inicialiado = 0;

int Sensor1_Time = 0;
int Sensor2_Time = 0;
int Same_Time = 0;

int Last_Sensor1_Time = 0;
int Last_Sensor2_Time = 0;
int Last_Same_Time = 0;

int Sensor1_Min_Time = 1;  // SISTEMA ANTI - RUIDO PARA
int Sensor2_Min_Time = 1;  // SISTEMA ANTI - RUIDO PARA
int Sametime_Min_Time = 0; // KOMATSU ANTI - RUIDO PARA

int delaymonitoreo = 0;

// int cont=0;

String A = "X";
String B = "X";
String C = "X";
String D = "X";

//=================================================================================================================================================
//============================================================== R U T I N E S ====================================================================
//=================================================================================================================================================
// Subrutina para el sensado de personas al salir o entrar || incluye filtro ABDC  || Modifica los valores de BDatos
unsigned int cont = 0;
char inicio = 0;
unsigned int contador_now = 0;

void censusPeople()
{
  cont++;
  // while (a=="a"){     //COMO PRUEBA PUESTO PARA ACELERAR LA LECTURA
  if (inicialiado == 0)
  {
    // total=0;
    // last_value = 0;
    cambio_pantalla = 1;
    inicialiado = 1;
  }

  ////////////////////////////////////////////////////////////
  // TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_1
  if (digitalRead(Sensor_1) == LOW)
  {
    Sensor1_Time++;
    // Serial.println("Sensor1_Time = "+ String(Sensor1_Time));
    if (Sensor1_Time != 0)
    {
      Last_Sensor1_Time = Sensor1_Time;
      // Serial.println(Last_Sensor1_Time);
    }
  }
  else
  {
    Sensor1_Time = 0;
  }

  // TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_2
  if (digitalRead(Sensor_2) == LOW)
  {
    Sensor2_Time++;
    // Serial.println("Sensor2_Time = "+ String(Sensor2_Time));
    if (Sensor2_Time != 0)
    {
      Last_Sensor2_Time = Sensor2_Time;
      // Serial.println(Last_Sensor2_Time);
    }
  }
  else
  {
    Sensor2_Time = 0;
  }

  // TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_2
  if (digitalRead(Sensor_1) == LOW && digitalRead(Sensor_2) == LOW)
  {
    Same_Time++;
    // Serial.println("Same_Time  = "+ String(Same_Time));
    if (Same_Time != 0)
    {
      Last_Same_Time = Same_Time;
      // Serial.println(Last_Same_Time);
    }
  }
  else
  {
    Same_Time = 0;
  }

  // Imp_Conteo_Tiempo_Real();
  // mpresion_ABCD();
  // Solo_Cont();

  // A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A -
  // A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A -
  // ASIGNANDO "A" SENSOR 1
  if (digitalRead(Sensor_1) == LOW && A == "X")
  {
    A = "1E";
  }
  // ASIGNANDO "A" SENSOR 2
  if (digitalRead(Sensor_2) == LOW && A == "X")
  {
    A = "2E";
  }

  // LIBERANDO "A" SENSOR 1
  if (digitalRead(Sensor_1) == HIGH && A == "1E" && B == "X" && C == "X" && D == "X")
  {
    A = "X";
  }

  // LIBERANDO "A" SENSOR 2
  if (digitalRead(Sensor_2) == HIGH && A == "2E" && B == "X" && C == "X" && D == "X")
  {
    A = "X";
  }
  // B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B -
  // B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B -
  // ASIGNANDO "B" SENSOR 1
  if (digitalRead(Sensor_1) == LOW && A == "2E")
  {
    B = "1E";
  }
  // ASIGNANDO "B" SENSOR 2
  if (digitalRead(Sensor_2) == LOW && A == "1E")
  {
    B = "2E";
  }

  // LIBERANDO "B" SENSOR 1
  if (digitalRead(Sensor_1) == HIGH && A == "2E" && B == "1E" && C == "X" && D == "X")
  {
    B = "X";
  }

  // LIBERANDO "B" SENSOR 2
  if (digitalRead(Sensor_2) == HIGH && A == "1E" && B == "2E" && C == "X" && D == "X")
  {
    B = "X";
  }
  // C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C -
  // C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C -
  // ASIGNANDO "C" SENSOR 1
  if (digitalRead(Sensor_1) == HIGH && A == "1E" && B == "2E")
  {
    C = "1S";
  }
  // ASIGNANDO "C" SENSOR 2
  if (digitalRead(Sensor_2) == HIGH && A == "2E" && B == "1E")
  {
    C = "2S";
  }

  // LIBERANDO "C" SENSOR 1
  if (digitalRead(Sensor_1) == LOW && A == "1E" && B == "2E" && C == "1S" && D == "X")
  {
    C = "X";
  }

  // LIBERANDO "C" SENSOR 2
  if (digitalRead(Sensor_2) == LOW && A == "2E" && B == "1E" && C == "2S" && D == "X")
  {
    C = "X";
  }

  // D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D -
  // D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D -

  // ASIGNANDO "D" SENSOR 1
  if (digitalRead(Sensor_1) == HIGH && A == "2E" && B == "1E" && C == "2S")
  {
    D = "1S";
  }
  // ASIGNANDO "D" SENSOR 2
  if (digitalRead(Sensor_2) == HIGH && A == "1E" && B == "2E" && C == "1S")
  {
    D = "2S";
  }

  if (digitalRead(Sensor_2) == HIGH && A == "1E" && B == "2E" && C == "1S")
  { // && B == "2E" C == "1S" && D == "2S"

    // FILTRO POR TIEMPO DE PULSO (TIEMPO SENSOR 1, TIEMPO SENSOR 2, AL MISMO TIEMPO
    if (Last_Sensor1_Time >= Sensor1_Min_Time && Last_Sensor2_Time >= Sensor2_Min_Time && Last_Same_Time >= Sametime_Min_Time)
    {

      // Serial.println ("ENTRÓ");

      // Serial.println ("PERSONA ENTRANDO");
      // Realiza la acción deseada

      // cli(); //Deshabilitar interrupciones
      ingreso++;
      total++;
      cambio_pantalla = 1;
    }
    else
    {
      Serial.println("CONTEO DE ENTRADA POR RUIDO, NO VÁLIDO");

      // Imp_Conteo_Ultimos_Tiempos();
    }

    A = "X";
    B = "X";
    C = "X";
    D = "X";
  }

  if (digitalRead(Sensor_1) == HIGH && A == "2E" && B == "1E" && C == "2S")
  { //&& B == "1E" && C == "2S" && D == "1S"

    // FILTRO POR TIEMPO DE PULSO (TIEMPO SENSOR 1, TIEMPO SENSOR 2, AL MISMO TIEMPO
    if (Last_Sensor1_Time >= Sensor1_Min_Time && Last_Sensor2_Time >= Sensor2_Min_Time && Last_Same_Time >= Sametime_Min_Time)
    {

      // Serial.println ("SALIÓ");

      // Serial.println ("PERSONA SALIENDO");
      // cli(); //Deshabilitar interrupciones
      egreso++;
      total--;
      cambio_pantalla = 1;

      // if (total < 0)
      //    {
      //     total = 0;
      //     egreso--;
      //    }
    }
    else
    {
      Serial.println("CONTEO DE SALIDA POR RUIDO, NO VÁLIDO");

      // IMPRESION DE ULTIMO TIEMPO REGISTRADO
      // Imp_Conteo_Ultimos_Tiempos();
    }

    A = "X";
    B = "X";
    C = "X";
    D = "X";
  }

  if (last_value != total)
  {
    if (last_value < total)
    {
      // si la persona entro
      //     {
      Serial.print("entro | "); // entro
      BDatos.total++;           // seteamos los valores locales
      BDatos.ingresos++;
      Serial.println("Aforo: " + String(BDatos.aforo) + "  Total: " + String(BDatos.total)); // entro
      // SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);    // el elemento separador es el |
      flag_send_data = 1; // enviamos la data actualizada
      // actualizar_pantalla();                                                                    // actualizamos pantalla con nuevos valores
      counter_minutos_inactividad = 0; // reset a minutos de inactividad
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
        counter_minutos_inactividad = 0;                                                     // reset a minutos de inactividad
        counter_millis = 0;                                                                  // reseteamos el counter_millis para anular el reseteo por inacctividad
      }                                                                                      // actualizamos pantalla con nuevos valores
      Serial.println("Aforo: " + String(BDatos.aforo) + "  Total: " + String(BDatos.total)); // entro
    }
    last_value = total;
    cambio_pantalla = 1;
  }

  if (cambio_pantalla == 1 && digitalRead(Sensor_1) == HIGH && digitalRead(Sensor_2) == HIGH)
  {
    // Pantalla();
    cambio_pantalla = 0;

    // IMPRESION DE ULTIMO TIEMPO REGISTRADO
    // Imp_Conteo_Ultimos_Tiempos();
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
// void reinicio_de_cuenta(){
//   if (hour() == hora_reinicio && minute()==minuto_reinicio && second() ==0 ){
//     Serial.print("Reinicio de Cuenta ------ día nuevo\n");
//     BDatos.total=0;
//     BDatos.ingresos=0;
//     BDatos.egresos=0;
//   }
//   minuto_anterior = minuto;
//   minuto = minute();
//   if (minuto_anterior != minuto){
//     counter_minutos_inactividad++;
//   }
//   if (counter_minutos_inactividad > minutos_inactividad_max){
//     Serial.print("Reinicio de Cuenta ----- por minutos de inactividad\n");
//     counter_minutos_inactividad = 0;
//     BDatos.total=0;
//     BDatos.ingresos=0;
//     BDatos.egresos=0;
//   }

// }

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
  Serial.begin(9600);
  //----------------------------------------DMDESP Display Setup
  // Disp.start(); //--> Run the DMDESP library
  // Disp.setBrightness(250); //--> Brightness level
  // Disp.setFont(FONT); //--> Determine the font used
  //----------------------------------------
  pinMode(PtoBuzzer, OUTPUT); // BUZZER
  delay(2);
  digitalWrite(PtoBuzzer, LOW);
  //-------------------------------------- SENSORES
  pinMode(Sensor_1, INPUT); // sets the digital pin as Input
  // attachInterrupt(digitalPinToInterrupt(Sensor_1), in_people, RISING);

  pinMode(Sensor_2, INPUT); // sets the digital pin as Input
  // attachInterrupt(digitalPinToInterrupt(Sensor_2), out_people, RISING);
  //-------------- WIFI UDP -----------------

  ConnectWiFi_STA();
  ConnectUDP();

  //--------------- CLOCK NTP SERVER ----------------
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

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
  EEPROM.begin(260);
  setCofigEprom ();
  jsonConfigDataSet();
}

int inicio_r = 0;
unsigned int refresh = 0; // variable para enviar datos cada 2 segundos, sistema de defensa frente a perdida de datos udp
time_t prevDisplay = 0;   // when the digital clock was displayed
unsigned long int PrevMillis = 0;

void loop()
{
  GetUDP_Packet(false); // esperar a que llegen paquetes

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

  if (((Firebase.ready() && (millis() - PrevMillis > 5000)) || PrevMillis == 0))
  {
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
    Serial.printf("Update json... %s\n\n", Firebase.RTDB.updateNodeAsync(&fbdo, path_data + fbdo.pushName(), &jsonData) ? "ok" : fbdo.errorReason().c_str());

    // int aforo_realtime = 0;
    // if (Firebase.RTDB.getInt(&fbdo, F("/tasa/callao/bano-varones/config/aforo/int")))
    // {
    //   aforo_realtime = (fbdo.to<int>());
    //   Serial.println("Aforo realtime = " + String(aforo_realtime));
    // }
    // else
    // {
    //   Serial.println(fbdo.errorReason().c_str());
    // }
  }
  censusPeople(); // Sesado de personas saliendo o ingresado. Actualiza BDatos.local y lo envia al otro esp
  refresh++;
  if (refresh == 65000)
  {
    refresh = 0;
    flag_send_data = 1;
  }

  Serial.println();
  Serial.println(readStringFromEEPROM(0));
  Serial.println(readStringFromEEPROM(50));
  Serial.println(readStringFromEEPROM(100));
  Serial.println(readStringFromEEPROM(150));
  Serial.println(readStringFromEEPROM(160));
  Serial.println(readStringFromEEPROM(170));
  Serial.println(readStringFromEEPROM(180));
  Serial.println(readStringFromEEPROM(190));
  Serial.println(readStringFromEEPROM(200));
  Serial.println(readStringFromEEPROM(210));
  Serial.println(readStringFromEEPROM(220));
  Serial.println(readStringFromEEPROM(230));
  Serial.println(readStringFromEEPROM(240));
  delay(15000);
}