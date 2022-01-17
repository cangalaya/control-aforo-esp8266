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
#include <WiFiUdp.h>              // Protocolo UDP (transferencia de datos rapida, pero no segura)
//#include <DMDESP.h>               // librería para el display
#include <Separador.h>            // liberia para separa string (se usa para separar valores en la trama de datos de comunicación con el UDP)

//#include <fonts/SystemFont5x7.h>  //liberias para la tipología de fuente a mostrar en pantalla
//#include <fonts/Droid_Sans_12.h>
//#include <fonts/Arial_Black_16.h>
//#include <fonts/Arial14.h>

//--------- Fuentes para el Display ------------------

//const uint8_t *FONT1 = Arial14;
//const uint8_t *FONT2 = Arial_Black_16;
//const uint8_t *FONT3 = SystemFont5x7;
//const uint8_t *FONT4 = Droid_Sans_12;
//const uint8_t *FONT = FONT2;

///// VARIABLES PARA EL RESET DE AFORO ////
int minuto = 0;
int minuto_anterior = 0;
int counter_minutos_inactividad = 0;

//----------------------------------------DMD Configuration (P10 Panel)
//#define DISPLAYS_WIDE 3 //--> Panel Columns         <<<<<<<<<<<<<<< D I S P L A Y   C O L U M N A S <<<<<<<<<<<
//#define DISPLAYS_HIGH 1 //--> Panel Rows
//DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);          //Creación de Instancia llamada Disp -> Number of Panels P10 used (Column, Row)
//----------------------------------------    
Separador s;                                        // instancia para separar string
//----------------------------------------


#include "config.h"                // Configurar datos de la red
#include "UDP.hpp"
#include "ESP8266_Utils.hpp"
#include "ESP8266_Utils_UDP.hpp"

//////////// variables para reseteo por inactividad /////////////
unsigned long time_millis = 0;
unsigned long before_time_millis= 0;
unsigned long counter_millis = 0;

//========== variables para la subrutina - censusPeople() ===========


byte Sensor_1 = LOW;
byte Sensor_2 = LOW;
#define Sensor_1  4
#define Sensor_2  5
#define PtoBuzzer 2             /////////<<<<<<<<< SALIDA -> BUZZER



int cambio_pantalla=0;


int inicialiado=0;

int Sensor1_Time=0;
int Sensor2_Time=0;
int Same_Time=0;

int Last_Sensor1_Time=0;
int Last_Sensor2_Time=0;
int Last_Same_Time=0;

int Sensor1_Min_Time=1;   // SISTEMA ANTI - RUIDO PARA 
int Sensor2_Min_Time=1;   // SISTEMA ANTI - RUIDO PARA 
int Sametime_Min_Time=0;  // KOMATSU ANTI - RUIDO PARA 

int delaymonitoreo=0;

//int cont=0;

String A ="X";
String B ="X";
String C ="X";
String D ="X";



//=================================================================================================================================================
//============================================================== R U T I N E S ====================================================================
//=================================================================================================================================================
//Subrutina para el sensado de personas al salir o entrar || incluye filtro ABDC  || Modifica los valores de BDatos
unsigned int cont = 0;
char inicio = 0;
unsigned int contador_now=0;

void censusPeople()
  {
  cont++;
 //while (a=="a"){     //COMO PRUEBA PUESTO PARA ACELERAR LA LECTURA  
  if (inicialiado==0){
  //total=0;  
  //last_value = 0;
  cambio_pantalla = 1;
  inicialiado=1;
  }

////////////////////////////////////////////////////////////
//TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_1
  if (digitalRead(Sensor_1) == LOW) 
  {      
    Sensor1_Time++;
    //Serial.println("Sensor1_Time = "+ String(Sensor1_Time));
    if (Sensor1_Time != 0)
    {
        Last_Sensor1_Time = Sensor1_Time;
        //Serial.println(Last_Sensor1_Time);
    }  
  }
  else
  { 
    Sensor1_Time=0;
  }
       
//TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_2
  if (digitalRead(Sensor_2) == LOW) 
  {
    Sensor2_Time++;
    //Serial.println("Sensor2_Time = "+ String(Sensor2_Time));
    if (Sensor2_Time != 0)
    {
      Last_Sensor2_Time = Sensor2_Time;
      //Serial.println(Last_Sensor2_Time);
    }
  }
  else
  {        
    Sensor2_Time=0;
  }

//TIEMPO (NO EXACTO ES SOLO UN CONTADOR) PARA EL LOW DEL SENSOR_2
    if (digitalRead(Sensor_1) == LOW && digitalRead(Sensor_2) == LOW) {       
      Same_Time++;
      //Serial.println("Same_Time  = "+ String(Same_Time));
          if (Same_Time != 0){
          Last_Same_Time = Same_Time;
          //Serial.println(Last_Same_Time);
          } 
      }
      else{ 
      Same_Time=0;
      }
     

      //Imp_Conteo_Tiempo_Real();
      //mpresion_ABCD();
      //Solo_Cont();


//A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A -
//A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A - A -
  //ASIGNANDO "A" SENSOR 1
 if (digitalRead(Sensor_1) == LOW && A == "X") {
    A="1E";
 }
  //ASIGNANDO "A" SENSOR 2
  if (digitalRead(Sensor_2) == LOW && A == "X") {
    A="2E";
 }

   //LIBERANDO "A" SENSOR 1
   if (digitalRead(Sensor_1) ==  HIGH && A == "1E" && B == "X" && C == "X" && D == "X") {
    A="X";
   }

    //LIBERANDO "A" SENSOR 2
   if (digitalRead(Sensor_2) ==  HIGH && A == "2E" && B == "X" && C == "X" && D == "X") {
    A="X";
   }
//B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B -
//B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B - B -
  //ASIGNANDO "B" SENSOR 1
 if (digitalRead(Sensor_1) == LOW && A == "2E") {
    B="1E";
 }
  //ASIGNANDO "B" SENSOR 2
  if (digitalRead(Sensor_2) == LOW && A == "1E") {
    B="2E";
 }

     //LIBERANDO "B" SENSOR 1
   if (digitalRead(Sensor_1) ==  HIGH && A == "2E" && B == "1E" && C == "X" && D == "X") {
    B="X";
   }

    //LIBERANDO "B" SENSOR 2
   if (digitalRead(Sensor_2) ==  HIGH && A == "1E" && B == "2E" && C == "X" && D == "X") {
    B="X";
   }
//C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C -   
//C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - C - 
     //ASIGNANDO "C" SENSOR 1
 if (digitalRead(Sensor_1) == HIGH && A == "1E" && B == "2E") {
    C="1S";
 }
     //ASIGNANDO "C" SENSOR 2
 if (digitalRead(Sensor_2) == HIGH && A == "2E" && B == "1E") {
    C="2S";
 }

      //LIBERANDO "C" SENSOR 1
   if (digitalRead(Sensor_1) ==  LOW && A == "1E" && B == "2E" && C == "1S" && D == "X") {
    C="X";
   }

    //LIBERANDO "C" SENSOR 2
   if (digitalRead(Sensor_2) ==  LOW && A == "2E" && B == "1E" && C == "2S" && D == "X") {
    C="X";
   }

 //D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D -
 //D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D - D -

     //ASIGNANDO "D" SENSOR 1
 if (digitalRead(Sensor_1) == HIGH && A == "2E" && B == "1E" && C == "2S") {
    D="1S";
 }
     //ASIGNANDO "D" SENSOR 2
 if (digitalRead(Sensor_2) == HIGH && A == "1E" && B == "2E" && C == "1S") {
    D="2S";
 }



  if (digitalRead(Sensor_2) == HIGH && A == "1E" && B == "2E" && C == "1S") {    // && B == "2E" C == "1S" && D == "2S"

      //FILTRO POR TIEMPO DE PULSO (TIEMPO SENSOR 1, TIEMPO SENSOR 2, AL MISMO TIEMPO
     if (Last_Sensor1_Time >= Sensor1_Min_Time && Last_Sensor2_Time >= Sensor2_Min_Time && Last_Same_Time >= Sametime_Min_Time){
      
   //Serial.println ("ENTRÓ");


     // Serial.println ("PERSONA ENTRANDO");
         //Realiza la acción deseada
            
          //cli(); //Deshabilitar interrupciones
          ingreso++;    
          total++;             
          cambio_pantalla = 1;
     }
  else{
     Serial.println ("CONTEO DE ENTRADA POR RUIDO, NO VÁLIDO");

      //Imp_Conteo_Ultimos_Tiempos();
      
  }     
   
    A="X";
    B="X";
    C="X";
    D="X";
  }


  if (digitalRead(Sensor_1) == HIGH && A == "2E" && B == "1E" && C == "2S") {    //&& B == "1E" && C == "2S" && D == "1S"

     //FILTRO POR TIEMPO DE PULSO (TIEMPO SENSOR 1, TIEMPO SENSOR 2, AL MISMO TIEMPO
     if (Last_Sensor1_Time >= Sensor1_Min_Time && Last_Sensor2_Time >= Sensor2_Min_Time && Last_Same_Time >= Sametime_Min_Time){

      
    //Serial.println ("SALIÓ");

 // Serial.println ("PERSONA SALIENDO");
        //cli(); //Deshabilitar interrupciones
        egreso++;    
        total--;     
        cambio_pantalla = 1;
         
   //if (total < 0)
   //   {
   //    total = 0;
   //    egreso--;  
   //   }      
    
  }
  else{
   Serial.println ("CONTEO DE SALIDA POR RUIDO, NO VÁLIDO");

         //IMPRESION DE ULTIMO TIEMPO REGISTRADO
          //Imp_Conteo_Ultimos_Tiempos();
   }
    
    A="X";
    B="X";
    C="X";
    D="X";
    
  }

                    if (last_value != total){
                    if (last_value < total)
                      {
                          // si la persona entro
                          //     {
                                Serial.print("entro | ");      //entro
                                BDatos.total ++;                // seteamos los valores locales
                                BDatos.ingresos ++;
                                Serial.println("Aforo: "+ String(BDatos.aforo) + "  Total: "+ String(BDatos.total));      //entro
                                //SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);    // el elemento separador es el |
                                flag_send_data = 1;     //enviamos la data actualizada
                                //actualizar_pantalla();                                                                    // actualizamos pantalla con nuevos valores
                                counter_minutos_inactividad = 0;    //reset a minutos de inactividad
                                counter_millis = 0;         // reseteamos el counter_millis para anular el reseteo por inacctividad
                      }
                     else
                      {
                                Serial.print("salio | ");      //salio
                                if (BDatos.total - 1 >= 0)      // protección para que no adopte valores negativos
                                {
                                  BDatos.total --;
                                  BDatos.egresos ++;
                                  //SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
                                  flag_send_data = 1;     //enviamos la data actualizada
                                  //actualizar_pantalla();
                                  counter_minutos_inactividad = 0; //reset a minutos de inactividad   
                                  counter_millis = 0;         // reseteamos el counter_millis para anular el reseteo por inacctividad  
                                }                                                             // actualizamos pantalla con nuevos valores
                                Serial.println("Aforo: "+ String(BDatos.aforo) + "  Total: "+ String(BDatos.total));      //entro
                      }
                    last_value = total;
                    cambio_pantalla = 1;
                    }

                     if (cambio_pantalla == 1  && digitalRead(Sensor_1) == HIGH && digitalRead(Sensor_2) == HIGH) {
                     //Pantalla();
                     cambio_pantalla = 0;
                     
                     //IMPRESION DE ULTIMO TIEMPO REGISTRADO
                     //Imp_Conteo_Ultimos_Tiempos();
                     }
}

//===============================================================================================================================================
//===============================================================================================================================================


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
  pinMode(PtoBuzzer, OUTPUT);             //BUZZER
  delay(2);
  digitalWrite(PtoBuzzer, LOW);
  //-------------------------------------- SENSORES
  pinMode(Sensor_1, INPUT); // sets the digital pin as Input
  //attachInterrupt(digitalPinToInterrupt(Sensor_1), in_people, RISING);
       
  pinMode(Sensor_2, INPUT);      // sets the digital pin as Input
  //attachInterrupt(digitalPinToInterrupt(Sensor_2), out_people, RISING); 
  //-------------- WIFI UDP -----------------
  
  ConnectWiFi_STA();
  ConnectUDP();


}




int inicio_r = 0;
unsigned int refresh = 0;                               // variable para enviar datos cada 2 segundos, sistema de defensa frente a perdida de datos udp
time_t prevDisplay = 0; // when the digital clock was displayed

void loop() 
{
  GetUDP_Packet(false);                                  // esperar a que llegen paquetes
  
  time_millis = millis();
  if (time_millis < 500){ //cuando millis desborde despues de 50d aprox.
    before_time_millis = 0;         // volvemos el tiempo_aux a 0;
  }
  
  if (before_time_millis != time_millis){
    before_time_millis = time_millis;
    counter_millis++;
    //Serial.println("Segundos transcurridos: " + String(counter_millis));
  }
  if(counter_millis >= (3600000*horas_inactividad_max) ){       //(60000*horas_inactividad_max)    60000 = 1min    1hora(60min)= 3600000
    BDatos.total = 0;
    counter_millis = 0;
    Serial.println("Reseteo del total por tiempo de innactividad");
  }
  
  
  
  if (inicio_r == 0)                   //condicional puesta el setear datos cuando cualquiera de las 2 puertas se apaga repentinamente
  {
    inicio_r = 1;
    //actualizar_pantalla();
    mandar_data_display();              //<<<<<<<<
    //BDatos.estado_inicial = 1;                      //salimos del estado incial
    SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    BDatos.estado_inicial = 1;                      //salimos del estado incial
  }

  if (flag_send_data == 1)                          // condicional para enviar datos iniciales
  {
    flag_send_data = 0;
    SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    ////////////////////////////// ENVIO DE DATOS AL DASHBOARD //////////////////////////////
    SendUDP_Packet_Dashboard(String() + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
    mandar_data_display();              //<<<<<<<<
    Serial.println("Datos Enviados");
  }
  
  
  
  //if (actualizar_disp == 1)                         // condicional para actualizar display
  //{
  //  actualizar_disp = 0;
  //  actualizar_pantalla();
  //}
  
  censusPeople();                               // Sesado de personas saliendo o ingresado. Actualiza BDatos.local y lo envia al otro esp

  refresh ++;                                  
  if (refresh == 65000)
  {
    refresh = 0;
    flag_send_data = 1;
  }


}
