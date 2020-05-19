#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#define TFT_CS 10
#define TFT_DC 9

// which analog pin to connect
#define THERMISTORPIN1 A2
#define THERMISTORPIN2 A3         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 247000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 26   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 4336
// the value of the 'other' resistor
#define SERIESRESISTOR 27000   

#define ONE_WIRE_BUS 1                       //D1 pin of nodemcu

OneWire oneWire(ONE_WIRE_BUS);
 
DallasTemperature sensors(&oneWire);            // Pass the oneWire reference to Dallas Temperature.

int samples[NUMSAMPLES];

/*El proceso se define de la siguiente manera:
 * 
 *1. Al encender se llena el evaporador y se enciende el calentador
 *    (Water_fill, Water_pump, Heater) variables
 * 
 * 2. Al presionar Start se enciende la bomba de vacio
 *    (Air_pump, Air_in, Vaccum_valve)
 *    
 * 3.Se deja pasar el vapor a la camara
 *    (Steam_in)
 *    
 * 4.Se abre y se cierra la valvula Steam_in para que se mantenga la temperatura
 *  si a temperatura va por debajo se enciende la valvula
 * 
 * 5.Cuando finaliza la esterilizacion se abre la valvula de salida
 *    (Exhaust_valve)
 * 
 * 6.Cuando la temperatura cae debajo de 106°C se comienza el secado
 *    (Vaccum_valve, Air_pump, Water_fill)
 *   Cuando se hace vacio se llena de aire
 *    (Air_in)
 *    
 * 7.Al completar todo se apaga
 * 
*/
unsigned int FRAME_X = 20;
unsigned int FRAME_Y = 30;
#define FRAME_W 80
#define FRAME_H 30

byte Water_fill = 0;
byte Steam_in = 2;
byte Air_in = 3;
byte Exhaust_valves = 4;
byte Vaccum_valve = 5;
byte Air_pump = 6;

bool Water_fill_status = false;
bool Steam_in_status = false;
bool Air_in_status = false;
bool Exhaust_valves_status = false;
bool Vaccum_valve_status = false;
bool Air_pump_status = false;
bool Heater_status = false;
bool Water_pump_status = false;

#define Water_low 4
#define Water_high 3
#define Door_sensor 2
#define Heater 5
#define Water_pump 6
 

Adafruit_MCP23017 mcp;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

unsigned long previousMillis=0;
unsigned long interval=0;

const int max_temp=150;
const float max_pressure=245.166;
//const float vaccum_value=-13.3322;
const float vaccum_value=2.5;
float Chamber_pressure=0;
float Jacket_pressure=26.12;
float Chamber_temp=24.54;
float Jacket_temp=0;
int subproceso=0;
int Objects_temps[5]={100,60,50,70,4};
bool Done;
int Cycles=0;
bool Dry_process=false;
bool Sterilized=false;
const float To= 26.12;
const int B=4033;
unsigned long RJ;
unsigned long RC;
const float Ro= 247000.0;
const float R1=20000.0;
int CTemp_reading=0;
int JTemp_reading=0;
bool stat=false;
bool pressure=false;
String Subprocesses[6]={"Llenado","Vacio","Esteriliz","Venteo","Venteo","Culminado"};




void drawFrame(){
  tft.fillScreen(ILI9341_WHITE);
 const byte pad=5;
 String List[]={"V.Agua:","V.Vapor:","V.Aire:","V.Escape","V.Vacio:","B.Aire:","B.Agua:","Calent:"};
 String List2[]={"E","A"};
 int i;
 int j;
 
  for (i = 0; i < 8; i = i + 1) {
    //tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, ILI9341_BLACK);
    //tft.fillRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, ILI9341_BLACK);
    //tft.drawRect(FRAME_X+pad, FRAME_Y+pad, FRAME_W-(2*pad), FRAME_H-(2*pad), ILI9341_BLACK);
    //tft.fillRect(FRAME_X+pad, FRAME_Y+pad, FRAME_W-(2*pad), FRAME_H-(2*pad), ILI9341_NAVY);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(FRAME_X + 6 , FRAME_Y+3);
    tft.setTextSize(2);
    tft.println(List[i]);

    tft.setCursor(FRAME_X + 120 , FRAME_Y+3);
    tft.setTextSize(2);
    
    switch(i){
      case 0:
      if(Water_fill_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      break;

      case 1:
      
      if(Steam_in_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;

      case 2:
      
      if(Air_in_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;
    

    case 3:
      
      if(Exhaust_valves_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;

      case 4:
      
      if(Vaccum_valve_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      break;

      case 5:
      
      if(Air_pump_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;

      case 6:
      
      if(Water_pump_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;

      case 7:
      
      if(Heater_status==true){
        tft.println(List2[0]);
      }
      else{
        tft.println(List2[1]);
      }
      
      break;
  }
  

    FRAME_Y+=20;   
  }
  
  FRAME_Y=30;
  int Box[4]={160,25,150,170};
  tft.drawRect(Box[0],Box[1],Box[2],Box[3], ILI9341_CYAN);
  tft.fillRect(Box[0],Box[1],Box[2],Box[3], ILI9341_BLACK);
  tft.drawRect(Box[0]+pad,Box[1]+pad,Box[2]-(2*pad),Box[3]-(2*pad), ILI9341_NAVY);
  tft.fillRect(Box[0]+pad,Box[1]+pad,Box[2]-(2*pad),Box[3]-(2*pad), ILI9341_NAVY);
  tft.setCursor(Box[0]+5,Box[1]+5);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Temp:");
  tft.setCursor(Box[0]+5,Box[1]+85);
  tft.println("Pres:");
  tft.setCursor(Box[0]+10,Box[1]+35);
  tft.println(Chamber_temp);
  //Serial.println(CTemp_reading);
  //Serial.println(JTemp_reading);
  tft.setCursor(Box[0]+10,Box[1]+55);
  tft.println( Subprocesses[subproceso]);

  tft.setCursor(Box[0]+10,Box[1]+115);
  tft.println(Chamber_pressure);
  tft.setCursor(Box[0]+10,Box[1]+135);
  tft.println(Jacket_pressure);
  delay(500);
}


void Proceso(){
int Tiempo=2;
int Object_temp=35;
interval=Tiempo*1000*60;
//2.5kgf/cm2  max val for pressure
//150°C is max temp
//vaccum is -100mmHg





    switch(subproceso){
      
      case 0:    //Llenado de evaporador
        Done=false;
        if(digitalRead(Door_sensor)==LOW){


            if( ( (digitalRead(Water_low)==HIGH) && (digitalRead(Water_high)==HIGH) ) || ( (digitalRead(Water_high)==HIGH)&& (digitalRead(Water_low)==LOW) )  ){
              mcp.digitalWrite(Water_fill,HIGH);
              digitalWrite(Water_pump,HIGH);
              
              Water_fill_status = true;
              Water_pump_status = true;
              
            }
            
            if(digitalRead(Water_high)==LOW){
              mcp.digitalWrite(Water_fill,LOW);
              digitalWrite(Water_pump,LOW);
              Water_fill_status = false;
              Water_pump_status = false;
              subproceso=1;  
            }
            
            
        }  
        else{
              mcp.digitalWrite(Water_fill,LOW);
              digitalWrite(Water_pump,LOW);
              digitalWrite(Heater,LOW);
              
              Water_fill_status = false;
              Heater_status = false;                   
              Water_pump_status = false;
        }
            
        digitalWrite(Heater,LOW);
        
        Heater_status = false;
      
       break;

       case 1:  //Creacion de Vacio

         if(digitalRead(Door_sensor)==LOW){

            //(Air_pump, Air_in, Vaccum_valve)
            if((Chamber_pressure>vaccum_value)&&(pressure==false)){
            mcp.digitalWrite(Air_pump,HIGH);
            mcp.digitalWrite(Water_fill,HIGH);
            mcp.digitalWrite(Vaccum_valve,HIGH);
            mcp.digitalWrite(Air_in,LOW);
            
            Water_fill_status = true;
            Air_in_status = false;
            Vaccum_valve_status = true;
            Air_pump_status = true;
            }
            if(Cycles<=2){
              
              if((Chamber_pressure<vaccum_value)&&(pressure==false)){
                mcp.digitalWrite(Air_pump,LOW);
                mcp.digitalWrite(Water_fill,LOW);
                mcp.digitalWrite(Vaccum_valve,LOW);
                mcp.digitalWrite(Air_in,HIGH);
                
            Water_fill_status = false;
            Air_in_status = true;
            Vaccum_valve_status = false;
            Air_pump_status = false;
                
                Cycles++;
                pressure=true;
              }

              if(pressure==true){
                mcp.digitalWrite(Air_in,HIGH);
                Air_in_status = true;
                if(Chamber_pressure>5){
                  pressure=false;
                }
              }
            }
            if((Cycles>2)&&(Dry_process==false)){
              
              Cycles=0;
              subproceso=2;
                mcp.digitalWrite(Air_pump,LOW);
                mcp.digitalWrite(Water_fill,LOW);
                mcp.digitalWrite(Vaccum_valve,LOW);
                mcp.digitalWrite(Air_in,LOW);
                Water_fill_status = false;
                Air_in_status = false;
                Vaccum_valve_status = false;
                Air_pump_status = false;
            
            }
            
            if((Cycles>2)&&(Dry_process==true)){
              Cycles=0;
              subproceso=5;
              Dry_process=false;
            }
            
         }
         else{
                mcp.digitalWrite(Air_pump,LOW);
                mcp.digitalWrite(Water_fill,LOW);
                mcp.digitalWrite(Vaccum_valve,LOW);
                mcp.digitalWrite(Air_in,HIGH);
                
                Water_fill_status = false;
                Air_in_status = true;
                Vaccum_valve_status = false;
                Air_pump_status = false;
            
         }
         
         break;

        case 2: //Sterilization
        digitalWrite(Heater,HIGH);
        if(digitalRead(Door_sensor==LOW)){
          
          if(Chamber_temp<=Object_temp){
            mcp.digitalWrite(Steam_in,HIGH);
            Steam_in_status=true;
          }
          else{
            mcp.digitalWrite(Steam_in,LOW);
            Steam_in_status=false;
          }
          
          unsigned long currentMillis = millis();

          if (currentMillis - previousMillis >= 60000) {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            Sterilized=true;
            mcp.digitalWrite(Steam_in,LOW);
            Steam_in_status=false;
            subproceso=3;

          }
          else{
            Sterilized=false;
          }
        }
          break;

        case 3:  //Vent
            if(digitalRead(Door_sensor)==LOW){

              mcp.digitalWrite(Exhaust_valves,HIGH);
              digitalWrite(Heater,LOW);
              mcp.digitalWrite(Steam_in,LOW);
              mcp.digitalWrite(Water_fill,LOW);
              mcp.digitalWrite(Air_in,LOW);
              mcp.digitalWrite(Air_pump,LOW);
              mcp.digitalWrite(Vaccum_valve,LOW);

              Water_fill_status = false;
              Steam_in_status = false;
              Air_in_status = false;
              Exhaust_valves_status = true;
              Vaccum_valve_status = false;
              Air_pump_status = false;
              Heater_status = false;
              Water_pump_status = false;
              
              if( (digitalRead(Water_low)==HIGH) && (digitalRead(Water_high)==HIGH) ){
              subproceso=4;  
              }
              else{
                subproceso=3;
              }

              
            }
            break;
        case 4: //Dry

            if(digitalRead(Door_sensor)==LOW){

              subproceso=1;
              Dry_process=true;
            }
            break;
         case 5:

            if(digitalRead(Door_sensor)==LOW){
              mcp.digitalWrite(Exhaust_valves,LOW);
              digitalWrite(Heater,LOW);
              mcp.digitalWrite(Steam_in,LOW);
              mcp.digitalWrite(Water_fill,LOW);
              mcp.digitalWrite(Air_in,LOW);
              mcp.digitalWrite(Air_pump,LOW);
              mcp.digitalWrite(Vaccum_valve,LOW); 
              Water_fill_status = false;
              Steam_in_status = false;
              Air_in_status = false;
              Exhaust_valves_status = false;
              Vaccum_valve_status = false;
              Air_pump_status = false;
              Heater_status = false;
              Water_pump_status = false;
              
              Done=true;
              
              
            }
          break;



          
        }
    }


//  return Done;




void AnalogToDigital(){
  sensors.requestTemperatures(); 
  Chamber_temp=sensors.getTempCByIndex(0);
  
  const float factor=5/1023;
  //CTemp_reading=analogRead(2);
  //JTemp_reading=analogRead(3);
  Chamber_pressure = (float(analogRead(0)) - 163.84)/5.2666 ; //Kpa
  Jacket_pressure = (float(analogRead(1)) - 163.84)/5.2666 ;
  Chamber_pressure = Chamber_pressure; //to Kgf/cm2
  Jacket_pressure = Jacket_pressure; //to Kgf/cm2
  
  //RC=R1*((5.0/(CTemp_reading*factor)-1.0));
  //RJ=R1*((5.0/(JTemp_reading*factor)-1.0));
  //Chamber_temp= (1.0/To) + ( (1.0/B) * log(RC/Ro) );
  //Chamber_temp= 1/Chamber_temp;
  //Jacket_temp= (1.0/((1.0/To) + (1.0/B)*log(RJ/Ro)));
}




void setup()
{
  mcp.begin();
  mcp.pinMode(Water_fill, OUTPUT);
  mcp.pinMode(Steam_in, OUTPUT);
  
  mcp.pinMode(Air_in, OUTPUT);
  mcp.pinMode(Vaccum_valve, OUTPUT);
  
  mcp.pinMode(Exhaust_valves, OUTPUT);
  mcp.pinMode(Air_pump, OUTPUT);
  
  
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  tft.begin();
  sensors.begin();
  //Serial.begin(115200);
  tft.fillScreen(ILI9341_WHITE);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1); 
  
  //Serial.begin(9600);
}

void loop() {


 AnalogToDigital();
 
 drawFrame();
 if(Done==false){
 Proceso();
 
 }
 


}
