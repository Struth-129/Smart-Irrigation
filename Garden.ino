#include<BlynkSimpleEsp8266.h>
#include<ESP8266WiFi.h>
#define BLYNK_PRINT Serial

//automatic control parameters
#define DRY_SOIL 66
#define WET_SOIL 85
#define COLD_TEMP 12
#define HOT_TEMP 22
#define TIME_PUMP_ON 15
#define TIME_LAMP_ON 15

//timer
#define READ_BUTTONS_TM   1L  // definitions in seconds
#define READ_SOIL_TEMP_TM 2L
#define READ_SOIL_HUM_TM  10L
#define READ_AIR_DATA_TM  2L
#define SEND_UP_DATA_TM   10L
#define AUTO_CTRL_TM      60L 

#define SHOW_SET_UP 30

//DHT22
#define DHTPIN D3
#define DHTTYPE DHT22
float airHum=0;
float airTemp=0;

//Soil Moister
#define soilMoisterPin A0
#define soilMoisterVcc D8
int soilMoister=0;

//DS18B20 Temperature Sensor 
#define ONE_WIRE_BUS 14
float soilTemp;

//Relays
#define PUMP_PIN D6
#define LAMP_PIN D7
boolean pumpStatus=0;
boolean lampStatus=0;
//#define Read_Button 13;

/*Buttons
#define PUMP_ON_BUTTON D9
#define LAMP_ON_BUTTON D10
#define SENSORS_READ_BUTTON D4*/

WidgetLED PUMPs(V0);
WidgetLED PUMPa(V5);
WidgetLED LAMPs(V1);
WidgetLED LAMPa(V6);

#include<SimpleTimer.h>
SimpleTimer timer;

#include "DHT.h"
DHT dht(DHTPIN, DHTTYPE);

#include<OneWire.h> // DS18B20 temperature sensor
#include<DallasTemperature.h>

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char auth[]="ZM-E7f6Xa7OKfQ2zexxMCL4QwYRZ5hKg";
char ssid[]="Saket";
char pass[]="saket123";


void setup() 
{
 Serial.begin(115200);
 delay(10);
 Serial.println("Garden Bot getting Started");

 pinMode(PUMP_PIN,OUTPUT);
 pinMode(LAMP_PIN,OUTPUT);
// pinMode(PUMP_ON_BUTTON,INPUT_PULLUP);
// pinMode(LAMP_ON_BUTTON,INPUT_PULLUP);
// pinMode(SENSORS_READ_BUTTON,INPUT_PULLUP);
 pinMode(soilMoisterVcc,OUTPUT);
 
 Blynk.begin(auth,ssid,pass);
 dht.begin();
 DS18B20.begin();

 PUMPs.off();
 PUMPa.off();
 LAMPs.off();
 LAMPa.off();

 digitalWrite(PUMP_PIN,LOW);
 digitalWrite(LAMP_PIN,LOW);
 digitalWrite(soilMoisterVcc,LOW);

 //waitButtonPress(SHOW_SET_UP);
 startTimers();
}

//Sensors_Data

void getDhtData(void)
{
  float tempIni = airTemp;
  float humIni = airHum;
  airTemp = dht.readTemperature();
  airHum =dht.readHumidity();

  if(isnan(airHum) || isnan(airTemp))
  {
    Serial.println("Failed to read from DHT sensor! ");
    Blynk.notify("Failed to read from DHT sensor! ");
    airTemp = tempIni;
    humIni = humIni;
    return;
  }
}

//Soil_Moister_Data

void getSoilMoisterData(void)
{
  soilMoister = 0;
  digitalWrite( soilMoisterVcc,HIGH);
  delay(500);
  int n=3;
  for(int i=0; i<n; i++)
  {
    soilMoister=soilMoister+analogRead(soilMoisterPin);
    delay(150); 
  }
  digitalWrite(soilMoisterVcc, LOW);
  soilMoister = soilMoister/n;
  soilMoister =map(soilMoister,600,0,0,100); //LM393 on 5V
}

//Get SoilTemp sensor 

void getSoilTempData()
{
  DS18B20.requestTemperatures();
  soilTemp= DS18B20.getTempCByIndex(0);

  int newData=((soilTemp + 0.05)*10); //fix soilTemp value
  soilTemp = (newData/10.0);
}

//button
/*void waitButtonPress(int waitTime)
{
  long startTiming=millis();
  while(debounce(Blynk.digitalRead(Read_Button)))
  {
    if((millis()-startTiming) > (waitTime*1000))
    {
      break;
    }
  }
}

boolean debounce(int pin)
{
  boolean state;
  boolean previousState;
  const int debounceDelay=30;

  previousState=digitalRead(pin);
  for(int c=0; c<debounceDelay; c++)
  {
    delay(1);
    state=digitalRead(pin);
    if(state!=previousState)
    {
      c=0;
      previousState=state;
    }
  }
  return state;
}*/

void startTimers(void)
{
 // timer.setInterval(READ_BUTTONS_TM*1000,readLocalCmd);
  timer.setInterval(AUTO_CTRL_TM*1000,autoControlPlantation);
  timer.setInterval(READ_SOIL_TEMP_TM*1000,getSoilTempData);
  timer.setInterval(READ_AIR_DATA_TM*1000,getDhtData);
  timer.setInterval(READ_SOIL_HUM_TM*1000,getSoilMoisterData);
  timer.setInterval(SEND_UP_DATA_TM*1000,sendUptime);
}
void loop() 
{
  timer.run();
  Blynk.run();
}

//Read Commands

BLYNK_WRITE(3)
{
  int i=param.asInt();
  if(i==1)
  {
    pumpStatus=!pumpStatus;
    aplyCmd();
  }
}

BLYNK_WRITE(4)
{
  int i=param.asInt();
  if(i==1)
  {
    lampStatus=!lampStatus;
    aplyCmd();
  }
}

//read Local Commands

/*void readLocalCmd()
{
  boolean digiValue=debounce(PUMP_ON_BUTTON);
  if(!digiValue)
  {
    pumpStatus=!pumpStatus;
    aplyCmd();
  }

  digiValue=debounce(LAMP_ON_BUTTON);
  if(!digiValue)
  {
    lampStatus=!lampStatus;
    aplyCmd();
  }
}*/

//Receive Commands and act on actuators

void aplyCmd()
{
  if(pumpStatus==1)
  {
    Blynk.notify("GardenBot : PUMPON");
    digitalWrite(PUMP_PIN,HIGH);
    PUMPs.on();
    PUMPa.on();
  }
  else
    {
      digitalWrite(PUMP_PIN,LOW);
      PUMPs.off();
      PUMPa.off();
    }

   if(lampStatus==1)
   {
    Blynk.notify("GardenBot : LAMPON");
    digitalWrite(LAMP_PIN,HIGH);
    PUMPs.on();
    PUMPa.on(); 
   }
   else
   {
     digitalWrite(LAMP_PIN,LOW);
     PUMPs.off();
     PUMPa.off();
   }
}

//automatically control based on sensor

void autoControlPlantation(void)
{
  if(soilMoister < DRY_SOIL)
  {
    turnPumpOn();
  }

  if(airTemp < COLD_TEMP)
  {
    turnLampOn();
  }
}

//turn pumpon for a certain amount of time

void turnPumpOn()
{
  pumpStatus=1;
  aplyCmd();
  delay(TIME_PUMP_ON*1000);
  pumpStatus=0;
  aplyCmd();
}

//turn lampon for a certain amount of time

void turnLampOn()
{
  lampStatus=1;
  aplyCmd();
  delay(TIME_LAMP_ON*1000);
  lampStatus=0;
  aplyCmd();
}

void sendUptime()
{
  Blynk.virtualWrite(10, airTemp); //virtual pin V10
  Blynk.virtualWrite(11, airHum); // virtual pin V11
  Blynk.virtualWrite(12, soilMoister); // virtual pin V12
  Blynk.virtualWrite(13, soilTemp); //virtual pin V13
}
