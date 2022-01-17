const char* ssid     = "WF_AFORO";        //WF_AFORO_SAM
const char* password = "ss1d_4FoRo_T4s4";     //ss1d_4FoRo_T4s4
const char* hostname = "COMEDOR-2";     // cambiar hostname

IPAddress ip(192, 168, 1, 202);                   // Cambiar dirección IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

//// CONFIGURACIÓN DE DOBLE PUERTA ///////////
unsigned int master_port = 8892;
unsigned int second_port = 8893;

/////// IP DASHBOARD ////////
uint8_t remoteIP_dashboard[] = {10,228,34,20};      // IP FIJA DEL DASHBOARD

////// HORA DE REINICIO DE CUENTA /////

float horas_inactividad_max = 1.5;

//----------------------------------------------------
typedef struct struct_message   // estructura de Base de Datos para MAIN GATE - PUERTA PRINCIPAL
{
  int estado_inicial = 0;
  int aforo = 11;               //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< A F O R O
  int total = 0;
  int ingresos = 0;
  int egresos = 0;
} struct_message_main;

struct_message BDatos;           // instancia BDatos de Datos Locales - esto se muestra en el display y se comparte con el SECOD GATE

struct_message BDatosRecv;       // instancia BDatoRcev para recibir los datos del SECOD GATE


//====================================== SUBRUTINA PARA MANDAR DATOS DE AFORO AL DISPLAY (NANO)==========================================
    // comunicación serial con el arduino nano para mostrar valores en pantalla
void mandar_data_display()
{
  Serial.println((String)"/" + BDatos.aforo + "/" + BDatos.total + "/");
}
