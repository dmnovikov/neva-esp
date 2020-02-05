#include "neva.h"

const int pin_RX=5; // D1
const int pin_TX=4; //D2

ulong timer1;
ulong tim;

int avail;

Neva nevamodule;

int neva_good=0;


void setup() {
  
  nevamodule.init(pin_RX, pin_TX);

  Serial.begin(9600);
  
  delay(500);
  
  Serial.println("**********************"); 
  Serial.println("starting setup");
  Serial.println("**********************"); 

  timer1=millis();
  tim=millis();

  if(nevamodule.handshake_neva()==1) {
    Serial.println("NEVA GOOD, finish setup");
    neva_good=1; //good handshake
  }else{
      Serial.println("Error handshake");
      neva_good=0;
  }

}

int counter=0;


void loop() {
     
     if(millis()-tim >500){
      
      if(counter==0){ 
        Serial.println("************** Sendup ... ***********************");
      }

      if(counter==1){
        if(neva_good) nevamodule.get_obis_str(VOLTAGE);
      }

      if(counter==2){
         if(neva_good) nevamodule.get_obis_str(FREQ);
      }
      
      if(counter==3){
         if(neva_good) nevamodule.get_obis_str(CURRENT);
      }
      counter++;
      if(counter>10) counter=0;
      
     
      tim=millis();
      
     }

     
  /* 
  avail=neva.available();

  String _s4="";
  for(int i=1; i<=avail; i++){
    _b=neva.read();
    _s4+=char(_b);
     Serial.print("LOOP:");
     Serial.print(_b,HEX);
     Serial.println("("+ String(char(_b)) + ")" );
  }
  if(avail>0) Serial.println();
  
     


     avail=neva.available();

     
*/
}
