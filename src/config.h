IPAddress ip(192, 168, 1, 202); // Cambiar dirección IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// ============ CONFIGURACIÓN DEL EQUIPO ===========
// Configuración Tarjeta
String cliente = "tasa";
String sede = "callao";
String nombre_ambiente = "bano-varones";
unsigned int aforo_init = 30;
unsigned int inactivity_hours_reset = 3;
unsigned int count_dalay_milisegundos = 0;
unsigned int set_data_realtime_segundos = 60;
String abcd = "medium";
String estado = "on";
// Configuración Wifi
String ssid = "WF_P2_2";         //WF_AFORO_SAM
String password = "R420437015R"; //ss1d_4FoRo_T4s4

// Configuración UDP
unsigned int master_port = 8892;
unsigned int second_port = 8893;
unsigned int datalogger_port = 8888;
unsigned int datalogger_ip = 20;
//==================================================

String path_config = "/" + cliente + "/" + sede + "/" + nombre_ambiente + "/config";
String path_data = "/" + cliente + "/" + sede + "/" + nombre_ambiente + "/data";
// /////// IP DASHBOARD ////////
IPAddress remoteIP_dashboard(0,0,0,0); // declaración

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
  for(int i=0;i<stringSize;i++)
  {
    EEPROM.write(address+i, data[i]);
    EEPROM.commit();
  }
  EEPROM.write(address + stringSize,'\0');   //Add termination null character
  EEPROM.commit();
}

String readStringFromEEPROM(int address)
{
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k = EEPROM.read(address);
  while(k != '\0' && len < 100)   //Read until null character
  {
    k = EEPROM.read(address + len);
    data[len] = k;
    len++;
  }
  data[len]='\0';
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
      Serial.println(" -> " + item + " = " + String(fbdo.to<int >()).c_str());
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
    trataDeDatoString("wifi", "ssid", 0);         //
    trataDeDatoString("wifi", "password", 50);    //

    Serial.println(" UDP :");
    trataDeDatoInt("udp", "master-port", 150);    // OK
    trataDeDatoInt("udp", "second-port", 160);    // OK
    trataDeDatoInt("udp", "datalogger-port", 170);  // OK
    trataDeDatoInt("udp", "datalogger-ip", 180);    // OK

    Serial.println(" TARJET :");
    trataDeDatoInt("tarjet", "aforo", 190);                       // OK
    trataDeDatoString("tarjet", "abcd", 200);                     // OK
    trataDeDatoInt("tarjet", "count-delay-milisegundos", 210);    // OK
    trataDeDatoString("tarjet", "estado", 220);                   
    trataDeDatoFloat("tarjet", "inactivity-hours-reset", 230);    // OK flotante
    trataDeDatoInt("tarjet", "set-data-realtime-segundos", 240);  // OK
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
    if (EEPROM.read(170) == 255)
      writeStringToEEPROM(170, String(datalogger_port));
    if (EEPROM.read(180) == 255)
      writeStringToEEPROM(180, String(datalogger_ip));

    Serial.println("- UDP:");
    Serial.println("   - master port = " + readStringFromEEPROM(150));
    Serial.println("   - second port = " + readStringFromEEPROM(160));
    Serial.println("   - datalogger port = " + readStringFromEEPROM(170));
    Serial.println("   - datalogger ip = xx,xx,xx," + readStringFromEEPROM(180));

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

    Serial.println("- TARJET:");
    Serial.println("   - aforo init = " + readStringFromEEPROM(190));
    Serial.println("   - abcd = " + readStringFromEEPROM(200));
    Serial.println("   - count dalay milisegundos = " + readStringFromEEPROM(210));
    Serial.println("   - estado = " + readStringFromEEPROM(220));
    Serial.println("   - inactivity hours reset = " + readStringFromEEPROM(230));
    Serial.println("   - set data realtime segundos = " + readStringFromEEPROM(240));
  }
}
bool actualizarFlash = true;
void actualizarConfigFlash () {
  if (hour() != 7 && hour() != 8 && hour() != 9 && hour() != 11 && hour() != 12 && hour() != 13 && hour() != 14 && hour() != 15 && hour() != 17 && hour() != 18 && hour() != 19) {
    // horarios de actualización -> 0 -- 6 - 10 - 4pm - 12pm
    if (minute() == 0 && actualizarFlash){
      Serial.println("<<<<<   A C T U A L I Z A N D O   M E M O R I A   F L A S H   <<<<<");
      setCofigEprom();        // actualizamos valores
      actualizarFlash = false;
    } 
  }
  if (minute() != 0){
    actualizarFlash = true;
  }
}