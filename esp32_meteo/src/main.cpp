#include <Arduino.h>
#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <BH1750FVI.h>
#include <driver/adc.h>
#include "DHT.h"
#include <iarduino_Pressure_BMP.h>

#include <mqtt_ini.h>
#include <cl_ds18b20.h>
#include <cl_dht22.h>
#include <cl_bh1750fvi.h>
#include <cl_bme280.h>
#include <cl_wdir.h>
#include <cl_wspeed.h>
#include <cl_rg11.h>

#define DHTPIN              18
#define PIN_DS18B20         19
uint8_t PIN_BH1750FVI_ADDR  = 16;
#define PIN_WDIR            32
#define PIN_WSPEED          33
#define PIN_RG11            13
#define PIN_PWR_SENS        15

#define Bucket_Size         0.01

#define def_path              "meteo"
#define def_subpath_ds18b20   "ds18b20"
#define def_subpath_dht22     "dht22"
#define def_subpath_bh1750fvi "bh1750fvi"
#define def_subpath_bme280    "bme280"
#define def_subpath_wdir      "wdir"
#define def_subpath_wspeed    "wspeed"
#define def_subpath_rg11      "rg11"

mqtt_ini client( 
  "METEO",     // Client name that uniquely identify your device
   def_path);

#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
cl_dht22 dht22(&dht, &client, String(def_subpath_dht22), 30000, false);

OneWire oneWire(PIN_DS18B20);
DallasTemperature DS18B20(&oneWire);
cl_ds18b20 ds18b20(&DS18B20, &client, String(def_subpath_ds18b20), 30000, false);

uint32_t LastRead_bh1750fvi = 0; //запоминаем время последней публикации
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI _BH1750FVI(PIN_BH1750FVI_ADDR, DEVICEADDRESS, DEVICEMODE);
cl_bh1750fvi bh1750fvi(&_BH1750FVI, &client, String(def_subpath_bh1750fvi), 30000, false);

iarduino_Pressure_BMP bmp(0x76);
cl_bme280 bme280(&bmp, &client, String(def_subpath_bme280), 30000, false);

cl_wdir wdir(ADC1_CHANNEL_4, &client, String(def_subpath_wdir), 1000, false);
cl_wspeed wspeed(ADC1_CHANNEL_5, &client, String(def_subpath_wspeed), 100, false);

cl_rg11 rg11(PIN_RG11, &client, String(def_subpath_rg11), Bucket_Size, false);

void setup() {
  Serial.begin(115200);
  Serial.println("");  Serial.println("Start!");

  pinMode(PIN_PWR_SENS, OUTPUT); digitalWrite(PIN_PWR_SENS, HIGH);

  dht22.begin();
  ds18b20.begin();
  bh1750fvi.begin();
  bme280.begin();
  wdir.begin();
  wspeed.begin();
  rg11.begin( );

  client.begin(true);

  sei();

  delay(200);
}

void onMsgCommand( const String &message ){
  rg11.command(message);  //[def_path]/[def_subpath_wspeed]/
}

void onConnection()
{
  dht22.subscribe( ); //[def_path]/[def_subpath_ds18b20]/interval
  ds18b20.subscribe( ); //[def_path]/[def_subpath_ds18b20]/interval
  bh1750fvi.subscribe( ); //[def_path]/[def_subpath_bh1750fvi]/interval
  bme280.subscribe( ); //[def_path]/[def_subpath_bh1750fvi]/interval
  wdir.subscribe( );  //[def_path]/[def_subpath_wdir]/interval
  wspeed.subscribe( );  //[def_path]/[def_subpath_wspeed]/interval
}

void OnCheckState(){
  dht22.loop( );
  ds18b20.loop( );
  bh1750fvi.loop( );
  bme280.loop( );
  wdir.loop( );
  wspeed.loop( );
  rg11.loop( );
 
  client.flag_start = false;
}

void OnLoad(){}

void loop() {
  client.loop( );
}