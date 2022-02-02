IPAddress ip(192, 168, 1, 202); // Cambiar dirección IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// ============ CONFIGURACIÓN DEL EQUIPO ===========
// Configuración Tarjeta
String cliente = "tasa";
String sede = "callao";
String nombre_ambiente = "comedor2";
unsigned int aforo_init = 28;
unsigned int inactivity_hours_reset = 2;
unsigned int count_dalay_milisegundos = 100;
unsigned int set_data_realtime_segundos = 5;
unsigned int logger_interval_minutes = 60;
String abcd = "medium";
String estado = "on";
// Configuración Wifi
String ssid = "WF_P2_2";         // WF_AFORO
String password = "R420437015R"; // ss1d_4FoRo_T4s4

// Configuración UDP
unsigned int master_port = 8890;
unsigned int second_port = 8891;
// unsigned int datalogger_port = 8888;
// unsigned int datalogger_ip = 20;
//==================================================

String path_config = "/" + cliente + "/" + sede + "/" + nombre_ambiente + "/config";
String path_data = "/" + cliente + "/" + sede + "/" + nombre_ambiente + "/data";
// /////// IP DASHBOARD ////////
// IPAddress remoteIP_dashboard(0, 0, 0, 0); // declaración
unsigned int localPort = master_port; // si falla la comunicación en 1 sentido. Cambiar el puerto, ya que puede que ya esta siendo usado por otro sistema
unsigned int remotePort = second_port;

// unsigned int remotePortDashboard = datalogger_port; /// puerto remoto del dashboard

////// HORA DE REINICIO DE CUENTA /////

float horas_inactividad_max = 1.5;
int hora_reinicio = 23;
int minuto_reinicio = 59;

//----------------------------------------------------
typedef struct struct_message // estructura de Base de Datos para MAIN GATE - PUERTA PRINCIPAL
{
  int estado_inicial = 0;
  int aforo = aforo_init; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< A F O R O
  int total = 0;
  int ingresos = 0;
  int egresos = 0;
  int excesos = 0;
} struct_message_main;

struct_message BDatos; // instancia BDatos de Datos Locales - esto se muestra en el display y se comparte con el SECOD GATE

struct_message BDatosRecv; // instancia BDatoRcev para recibir los datos del SECOD GATE

//====================================== SUBRUTINA PARA MANDAR DATOS DE AFORO AL DISPLAY (NANO)==========================================
// comunicación serial con el arduino nano para mostrar valores en pantalla
// ======================= FUNCIONES ESCRIBIR STRINGS EEPROM ===============
void writeStringToEEPROM(int address, String data)
{
  int stringSize = data.length();
  for (int i = 0; i < stringSize; i++)
  {
    EEPROM.write(address + i, data[i]);
    EEPROM.commit();
  }
  EEPROM.write(address + stringSize, '\0'); // Add termination null character
  EEPROM.commit();
}

String readStringFromEEPROM(int address)
{
  char data[100]; // Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(address);
  while (k != '\0' && len < 100) // Read until null character
  {
    k = EEPROM.read(address + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
// =========================================================================
void trataDeDatoString(String section, String item, int direccionMemoria)
{
  if (Firebase.RTDB.getString(&fbdo, path_config + "/" + section + "/" + item + "/"))
  {
    String datoLeidoRTBD = String(fbdo.to<const char *>());
    String datoLeidoMemoria = readStringFromEEPROM(direccionMemoria);
    Serial.println(" -> " + item + " = " + datoLeidoRTBD);
    if (datoLeidoMemoria != datoLeidoRTBD) // verificar si el dato es el mismo guardado en memoria
    {
      writeStringToEEPROM(direccionMemoria, String(datoLeidoRTBD));
      Serial.println("   - " + item + " nuevo en eeprom " + readStringFromEEPROM(direccionMemoria));
    }
  }
  else
  {
    Serial.println("** Error al recibir " + item + " : " + String(fbdo.errorReason().c_str()));
  }
}
void trataDeDatoInt(String section, String item, unsigned int direccionMemoria)
{
  if (Firebase.RTDB.getInt(&fbdo, path_config + "/" + section + "/" + item))
  {
    Serial.println(" -> " + item + " = " + String(fbdo.to<int>()).c_str());
    if (readStringFromEEPROM(direccionMemoria) != String(fbdo.to<int>())) // verificar si el dato es el mismo guardado en memoria
    {
      writeStringToEEPROM(direccionMemoria, String(fbdo.to<int>()));
      Serial.println("   - " + item + " nuevo en eeprom " + readStringFromEEPROM(direccionMemoria));
    }
  }
  else
  {
    Serial.println("** Error al recibir " + item + " : " + String(fbdo.errorReason().c_str()));
  }
}
void trataDeDatoFloat(String section, String item, unsigned int direccionMemoria)
{
  if (Firebase.RTDB.getFloat(&fbdo, path_config + "/" + section + "/" + item))
  {
    Serial.println(" -> " + item + " = " + String(fbdo.to<float>()).c_str());
    if (readStringFromEEPROM(direccionMemoria) != String(fbdo.to<float>())) // verificar si el dato es el mismo guardado en memoria
    {
      writeStringToEEPROM(direccionMemoria, String(fbdo.to<float>()));
      Serial.println("   - " + item + " nuevo en eeprom " + readStringFromEEPROM(direccionMemoria));
    }
  }
  else
  {
    Serial.println("** Error al recibir " + item + " : " + String(fbdo.errorReason().c_str()));
  }
}
// =========================================================================

void mandar_data_display()
{
  Serial.println((String) "/" + BDatos.aforo + "/" + BDatos.total + "/");
}

void setCofigEprom()
{
  // wifi
  if (Firebase.ready())
  {
    Serial.println();
    Serial.println("..OK FIREBASE  CONECTADO");
    Serial.println(" WIFI :");
    trataDeDatoString("wifi", "ssid", 0);      //
    trataDeDatoString("wifi", "password", 50); //

    Serial.println(" UDP :");
    trataDeDatoInt("udp", "master-port", 150); // OK
    trataDeDatoInt("udp", "second-port", 160); // OK

    Serial.println(" TARJET :");
    trataDeDatoInt("tarjet", "aforo", 190);                    // OK
    trataDeDatoString("tarjet", "abcd", 200);                  // OK
    trataDeDatoInt("tarjet", "count-delay-milisegundos", 210); // OK
    trataDeDatoString("tarjet", "estado", 220);
    trataDeDatoFloat("tarjet", "inactivity-hours-reset", 230);   // OK flotante
    trataDeDatoInt("tarjet", "set-data-realtime-segundos", 240); // OK
    trataDeDatoInt("tarjet","logger-interval-minutes", 250 );
  }
  else
  {

    Serial.println(">>>> !!! Firebase no conectado -> estableciendo valores por defecto o usando ya escritos");

    // WIFI
    if (EEPROM.read(0) == 255)
      writeStringToEEPROM(0, ssid); // si la eeprom en la dirección 0 esta vaćia, escribe el valor por defecto
    if (EEPROM.read(50) == 255)
      writeStringToEEPROM(50, password);

    Serial.println("- WIFI:");
    Serial.println("   - ssid = " + readStringFromEEPROM(0));
    Serial.println("   - password = " + readStringFromEEPROM(50));

    // UDP
    if (EEPROM.read(150) == 255)
      writeStringToEEPROM(150, String(master_port)); // ocupa 10 espacios
    if (EEPROM.read(160) == 255)
      writeStringToEEPROM(160, String(second_port));

    Serial.println("- UDP:");
    Serial.println("   - master port = " + readStringFromEEPROM(150));
    Serial.println("   - second port = " + readStringFromEEPROM(160));

    // TARJET
    if (EEPROM.read(190) == 255)
      writeStringToEEPROM(190, String(aforo_init));
    if (EEPROM.read(200) == 255)
      writeStringToEEPROM(200, abcd);
    if (EEPROM.read(210) == 255)
      writeStringToEEPROM(210, String(count_dalay_milisegundos));
    if (EEPROM.read(220) == 255)
      writeStringToEEPROM(220, estado);
    if (EEPROM.read(230) == 255)
      writeStringToEEPROM(230, String(inactivity_hours_reset));
    if (EEPROM.read(240) == 255)
      writeStringToEEPROM(240, String(set_data_realtime_segundos));
    if (EEPROM.read(250) == 255)
      writeStringToEEPROM(250, String(logger_interval_minutes));

    Serial.println("- TARJET:");
    Serial.println("   - aforo init = " + readStringFromEEPROM(190));
    Serial.println("   - abcd = " + readStringFromEEPROM(200));
    Serial.println("   - count dalay milisegundos = " + readStringFromEEPROM(210));
    Serial.println("   - estado = " + readStringFromEEPROM(220));
    Serial.println("   - inactivity hours reset = " + readStringFromEEPROM(230));
    Serial.println("   - set data realtime segundos = " + readStringFromEEPROM(240));
    Serial.println("   - logger interval minutes = " + readStringFromEEPROM(250));
  }
}

////// SETEO INICIAL DE DATOS //////////
void jsonConfigDataSetFirstStart()
{
  // SETEO DE CONFIGURACIÓN DE TARJET
  jsonConfigTarjet.add("abcd", abcd);
  jsonConfigTarjet.add("aforo", aforo_init);
  jsonConfigTarjet.add("count-delay-milisegundos", count_dalay_milisegundos); // falta configurar
  jsonConfigTarjet.add("estado", estado);                                     // falta configurar
  jsonConfigTarjet.add("inactivity-hours-reset", inactivity_hours_reset);
  jsonConfigTarjet.add("set-data-realtime-segundos", set_data_realtime_segundos); // falta configurar
  jsonConfigTarjet.add("logger-interval-minutes", logger_interval_minutes);

  // SETEO DE CONFIGURACIÓN WIFI
  jsonConfigWifi.add("ssid", ssid);
  jsonConfigWifi.add("password", password);
  jsonConfigWifi.add("MAC-address", String(WiFi.macAddress())); // dirección mac
  jsonConfigWifi.add("IP-address", String(String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3])));

  // SETEO DE CONFIGURACIÓN UDP
  jsonConfigUdp.add("master-port", master_port);
  jsonConfigUdp.add("second-port", second_port);

  // SETEO DE CONFIGURACIÓN DATA - REALTIME
  jsonData.add("egresos", BDatos.egresos);
  jsonData.add("excesos", BDatos.excesos);
  jsonData.add("ingresos", BDatos.ingresos);
  jsonData.add("total", BDatos.total);

  if (Firebase.RTDB.setJSONAsync(&fbdo, path_config + "/wifi", &jsonConfigWifi))
  {
    Serial.println();
    Serial.println("Datos WIFI Firebase Realtime establecidos !!");
  }
  else
  {
    Serial.println();
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

void jsonConfigDataSet()
{

  // SETEO DE CONFIGURACIÓN DE TARJET
  jsonConfigTarjet.add("abcd", readStringFromEEPROM(200));
  jsonConfigTarjet.add("aforo", readStringFromEEPROM(190).toInt());
  jsonConfigTarjet.add("count-delay-milisegundos", readStringFromEEPROM(210).toInt()); // falta configurar
  jsonConfigTarjet.add("estado", readStringFromEEPROM(220));                           // falta configurar
  jsonConfigTarjet.add("inactivity-hours-reset", readStringFromEEPROM(230).toFloat());
  jsonConfigTarjet.add("set-data-realtime-segundos", readStringFromEEPROM(240).toInt()); // falta configurar
  jsonConfigTarjet.add("logger-interval-minutes", readStringFromEEPROM(250).toInt());

  // SETEO DE CONFIGURACIÓN WIFI
  jsonConfigWifi.add("ssid", readStringFromEEPROM(0));
  jsonConfigWifi.add("password", readStringFromEEPROM(50));
  jsonConfigWifi.add("MAC-address", String(WiFi.macAddress())); // dirección mac
  jsonConfigWifi.add("IP-address", String(String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3])));

  // SETEO DE CONFIGURACIÓN UDP
  jsonConfigUdp.add("master-port", readStringFromEEPROM(150).toInt());
  jsonConfigUdp.add("second-port", readStringFromEEPROM(160).toInt());

  // SETEO DE CONFIGURACIÓN DATA - REALTIME
  jsonData.add("egresos", BDatos.egresos);
  jsonData.add("excesos", BDatos.excesos);
  jsonData.add("ingresos", BDatos.ingresos);
  jsonData.add("total", BDatos.total);

  if (Firebase.RTDB.setJSONAsync(&fbdo, path_config + "/wifi", &jsonConfigWifi))
  {
    Serial.println();
    Serial.println("Datos WIFI Firebase Realtime establecidos !!");
  }
  else
  {
    Serial.println();
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

void actualizarConfigFlash()
{

  Serial.println("<<<<<   A C T U A L I Z A N D O   M E M O R I A   F L A S H   <<<<<");
  setCofigEprom();                                  // actualizamos valores
  //jsonConfigDataSet();                              // enviamos actualización
  BDatos.aforo = readStringFromEEPROM(190).toInt(); // siempre actualizamos el aforo
  horas_inactividad_max = readStringFromEEPROM(230).toFloat();
  localPort = readStringFromEEPROM(150).toInt();
  remotePort = readStringFromEEPROM(160).toInt();
}