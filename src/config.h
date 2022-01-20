IPAddress ip(192, 168, 1, 202);                   // Cambiar dirección IP
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
String ssid     = "WF_P2_2";        //WF_AFORO_SAM
String password = "R420437015R";     //ss1d_4FoRo_T4s4
String hostname = "esp1";     // cambiar hostname
// Configuración UDP
unsigned int master_port = 8892;
unsigned int second_port = 8893;
unsigned int datalogger_port = 8888;
unsigned int datalogger_ip = 20;
//==================================================

String path_config  = "/"+ cliente + "/" + sede + "/" + nombre_ambiente + "/config";
String path_data = "/"+ cliente + "/" + sede + "/" + nombre_ambiente + "/data";
/////// IP DASHBOARD ////////
uint8_t remoteIP_dashboard[] = {10,228,34,20};      // IP FIJA DEL DASHBOARD Siempre tratar de que sea el 20

////// HORA DE REINICIO DE CUENTA /////

float horas_inactividad_max = 1.5;

//----------------------------------------------------
typedef struct struct_message   // estructura de Base de Datos para MAIN GATE - PUERTA PRINCIPAL
{
  int estado_inicial = 0;
  int aforo = 0;               //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< A F O R O
  int total = 0;
  int ingresos = 0;
  int egresos = 0;
} struct_message_main;

struct_message BDatos;           // instancia BDatos de Datos Locales - esto se muestra en el display y se comparte con el SECOD GATE

struct_message BDatosRecv;       // instancia BDatoRcev para recibir los datos del SECOD GATE


//====================================== SUBRUTINA PARA MANDAR DATOS DE AFORO AL DISPLAY (NANO)==========================================
    // comunicación serial con el arduino nano para mostrar valores en pantalla
// ======================= FUNCIONES ESCRIBIR STRINGS EEPROM ===============
void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\ 0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}
// =========================================================================


void mandar_data_display()
{
  Serial.println((String)"/" + BDatos.aforo + "/" + BDatos.total + "/");
}

void setCofigEprom () {
  // wifi
  if (Firebase.ready()){

    Serial.println("..OK FIREBASE  CONECTADO");

    Serial.println(" WIFI :");
    if (Firebase.RTDB.getString(&fbdo, "/tasa/callao/bano-varones/config/wifi/ssid/"))
    {
      Serial.println(" - ssid = " + String(fbdo.to<const char *>()));
    }else {
      Serial.println("** Error al recibir ssid : " + String(fbdo.errorReason().c_str()) );
    }
    if (Firebase.RTDB.getString(&fbdo, "/tasa/callao/bano-varones/config/wifi/password/"))
    {
      Serial.println(" - password = " + String(fbdo.to<const char *>()));
    }else {
      Serial.println("** Error al recibir password : " + String(fbdo.errorReason().c_str()) );
    }
    if (Firebase.RTDB.getString(&fbdo, "/tasa/callao/bano-varones/config/wifi/hostname/"))
    {
      Serial.println(" - hostname = " + String(fbdo.to<const char *>()));
    }else {
      Serial.println("** Error al recibir hostname : " + String(fbdo.errorReason().c_str()) );
    }
  }
  else {
    
    Serial.println(">>>> !!! Firebase no conectado -> estableciendo valores por defecto o usando ya escritos");
    
    // WIFI
    if (EEPROM.read(0) == 255) writeStringToEEPROM(0,ssid);   // si la eeprom en la dirección 0 esta vaćia, escribe el valor por defecto
    if (EEPROM.read(50) == 255) writeStringToEEPROM(50,password);
    if (EEPROM.read(100) == 255) writeStringToEEPROM(100,hostname);

    Serial.println("- WIFI:");
    Serial.println("   - ssid = " + ssid);
    Serial.println("   - password = " + password);
    Serial.println("   - hostname = " + hostname);

    // UDP
    if (EEPROM.read(150) == 255) writeStringToEEPROM(150,String(master_port));  // ocupa 10 espacios
    if (EEPROM.read(160) == 255) writeStringToEEPROM(160,String(second_port));
    if (EEPROM.read(170) == 255) writeStringToEEPROM(170,String(datalogger_port));
    if (EEPROM.read(180) == 255) writeStringToEEPROM(180,String(datalogger_ip));

    Serial.println("- UDP:");
    Serial.println("   - master port = " + master_port);
    Serial.println("   - second port = " + second_port);
    Serial.println("   - datalogger port = " + datalogger_port);
    Serial.println("   - datalogger ip = xx,xx,xx," + datalogger_ip);

    // TARJET
    if (EEPROM.read(190) == 255) writeStringToEEPROM(190,String(aforo_init));
    if (EEPROM.read(200) == 255) writeStringToEEPROM(200,abcd);
    if (EEPROM.read(210) == 255) writeStringToEEPROM(210,String(count_dalay_milisegundos));
    if (EEPROM.read(220) == 255) writeStringToEEPROM(220,estado);
    if (EEPROM.read(230) == 255) writeStringToEEPROM(230,String(inactivity_hours_reset));
    if (EEPROM.read(230) == 255) writeStringToEEPROM(230,String(set_data_realtime_segundos));

    Serial.println("- TARJET:");
    Serial.println("   - aforo init = " + master_port);
    Serial.println("   - abcd = " + abcd);
    Serial.println("   - count dalay milisegundos = " + datalogger_port);
    Serial.println("   - estado = " + datalogger_ip);
    Serial.println("   - inactivity hours reset = " + inactivity_hours_reset);
    Serial.println("   - set data realtime segundos = " + inactivity_hours_reset);
    
  }
}
