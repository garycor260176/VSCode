#include <mqtt_ini.h>
#include <Wire.h>
#include <Preferences.h>
#include <cl_vl53l1x.h>

uint16_t dist = 1700;

cl_vl53l1x Zones[2];
int Zone = 0;
int counter = 0;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  Zones[0].begin(18, 0x2A, dist);
  Zones[1].begin(19, 0x2B, dist);
  Serial.println("setup - done!");
}

int first = 0;
int counter_old = 0;

void loop()
{
  s_vl53l1x zone1 = Zones[0].getState();
  s_vl53l1x zone2 = Zones[1].getState();
  if(!zone1.val && !zone2.val) first = 0;
  int dir = 0;
  int counter = counter_old;

  if(Zones[Zone].read()) {
    zone1 = Zones[0].getState();
    zone2 = Zones[1].getState();

    if(first == 0) {
      if(zone1.val && !zone2.val) {
        first = 1;
      } else 
      if(!zone1.val && zone2.val) {
        first = 2;
      }
    }

    if(!zone1.val && !zone2.val && ( zone1.val_last || zone2.val_last)) {
      switch(first){
        case 1:
          if(
            zone1.t_up <= zone2.t_up &&
            zone2.t_up <= zone1.t_dn &&
            zone1.t_dn <= zone2.t_dn
          ) {
            dir = 1;
          }
        break;

        case 2:
          if(
            zone2.t_up <= zone1.t_up &&
            zone1.t_up <= zone2.t_dn &&
            zone2.t_dn <= zone1.t_dn
          ) {
            dir = 2;
          }
        break;
      }
    }
  } 

  switch(dir){
    case 1: counter++; break;
    case 2: counter--; break;
  }

  if(!zone1.val && !zone2.val) {
    Zones[0].clear();
    Zones[1].clear();
	first = 0;
  }

  if(counter_old != counter) Serial.println(counter);

  counter_old = counter;

  Zone++;
  if(Zone >= 2) Zone = 0;
}