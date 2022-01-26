// UDP variables
WiFiUDP UDP;  


char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,

int ingreso = 0;
int egreso = 0;
int total = 0;
int last_value = 0;
int diferencia = 0;

//char actualizar_disp = 0;
int flag_send_data = 0;

void ProcessPacket(String response)
{

    BDatosRecv.estado_inicial = s.separa(response,'|',0 ).toInt();   //separamos el estado inicial
    Serial.print("estado_inicial recv: ");
    Serial.println(BDatosRecv.estado_inicial);

    BDatosRecv.aforo = s.separa(response,'|',1 ).toInt();    //aforo re
    Serial.print("aforo recv: ");
    Serial.println(BDatosRecv.aforo);
  
    BDatosRecv.total = s.separa(response,'|',2 ).toInt();    //recibimos valores
    Serial.print("total recv: ");
    Serial.println(BDatosRecv.total);
  
    BDatosRecv.ingresos = s.separa(response,'|',3 ).toInt();   //recibimos valores
    Serial.print("ingresos recv: ");
    Serial.println(BDatosRecv.ingresos);

    BDatosRecv.egresos = s.separa(response,'|',4 ).toInt();  //recibimos valores
    Serial.print("egresos recv: ");
    Serial.println(BDatosRecv.egresos);
  

    if (BDatosRecv.estado_inicial == 0)                                   // si el receptor se reseteo repentinamente
    {
      delay(10);
      //SendUDP_Packet(String() + BDatos.estado_inicial + '|' + BDatos.aforo + '|' + BDatos.total + '|' + BDatos.ingresos + '|' + BDatos.egresos);
      flag_send_data = 1;                                               // envío datos iniciales
      //BDatos.total = 1;
      //total = 0;
      //last_value = 0;
      return;
    }
    
    if (BDatosRecv.aforo != BDatos.aforo)
    {
      BDatos.aforo = BDatosRecv.aforo;        // si el aforo cambia, entonces actualizamos el valor local
      Serial.println("**Aforo actualizado ");
    }
    if (BDatosRecv.total != BDatos.total)
    {
      //diferencia = BDatosRecv.total - BDatos.total;   //actualizamos los valores localea
      BDatos.total = BDatosRecv.total;
      
      counter_minutos_inactividad = 0;      // reseteamos el counter de innactividad
      
      Serial.print("Total actualizado ");
    }
    if (BDatosRecv.ingresos != BDatos.ingresos)
    {
      //diferencia = BDatosRecv.ingresos - BDatos.ingresos;   //actualizamos los valores localea
      BDatos.ingresos = BDatosRecv.ingresos;
      Serial.print(" ingresos actualizado ");
    }
    if (BDatosRecv.egresos != BDatos.egresos)
    {
      //diferencia = BDatosRecv.egresos - BDatos.egresos;   //actualizamos los valores localea
      BDatos.egresos = BDatosRecv.egresos;
      Serial.print(" egresos actualizado");
    }
    Serial.println("");
    Serial.println("Aforo: "+ String(BDatos.aforo) + "  Total: "+ String(BDatos.total));  //mostramos datos en pantalla
    mandar_data_display();
    //actualizar_pantalla();                          
    //actualizar_disp = 1;                                  //refrescamos pantalla
     
}
