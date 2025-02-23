#include <mqtt_ini.h>
#include <Preferences.h>

#define def_path "LD2450_01"
mqtt_ini client( 
  "LD2450_01",
   def_path);

struct struPoint {
  int x;
  int y;
};
   
struct struZone {
  struPoint LeftTop;
  struPoint RightBottom;
};

struct s_settings{
  struZone Zone1;
};
  
struct s_state{
  int Zone1_state;
};

s_state cur_state;
s_settings ini;
Preferences preferences;

#define INTERVAL 100
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33

long previousMillis = 0;
byte findheader = 0; 
int16_t Target1_angle = 0;

float tanAngle[] = {0.000, 0.035, 0.070, 0.105, 0.141, 0.176, 0.213, 0.249, 0.287, 0.325, 0.364, 0.404, 0.445, 0.488, 0.532, 0.577, 0.625, 0.675, 0.727, 0.781, 0.839, 0.900, 0.966, 1.036, 1.111, 1.192, 1.280, 1.376, 1.483, 1.600, 1.732};

struct Position {
  int16_t Target1_X;
  int16_t Target1_Y;
  int16_t Target1_angle;
};

Position GetCoord() {
  Position str;
  if(RADAR_SERIAL.available()){
    if (RADAR_SERIAL.read()==0xAA && RADAR_SERIAL.read()==0xFF) {
      findheader=1;
    } else {
      return str;
    }
  }
  if (findheader == 1 && RADAR_SERIAL.available()){
   byte h_dataX, l_dataX, h_dataY, l_dataY;
   byte buf[28];
   RADAR_SERIAL.readBytes(buf, 28);
   h_dataX = buf[2];
   l_dataX = buf[3];
   h_dataY = buf[4];
   l_dataY = buf[5];
   if (buf[27]==0xCC) {
    if((l_dataX >> 7) == 0x1){
      str.Target1_X = ((l_dataX & 0x7F) << 8) + h_dataX;  
    }
    else {
      str.Target1_X = 0-(((l_dataX & 0x7F) << 8) + h_dataX);  
    }
       
    if((l_dataY >> 7) == 0x1){
      str.Target1_Y = (((l_dataY & 0x7F) << 8) + h_dataY); 
    }
    else {
      str.Target1_Y = 0 - (((l_dataY & 0x7F) << 8) + h_dataY); 
    }
    
    float Target1_tan = float(str.Target1_X) / float(str.Target1_Y);

    int i = 0;
    while (tanAngle[i] < abs(Target1_tan)) {
      i = i+1;
    }
    if (Target1_tan >= 0){
      str.Target1_angle = i * 2;  
    }
    else {
      str.Target1_angle = 0 - (i * 2);
    }
   }
  else {
  }
   return str;
   findheader = 0; // в конце сбрасывая флаг найденного заголовка
  } 
  return str;
}

s_settings read_eeprom(){
  s_settings ret;
  ret.Zone1.LeftTop.x = -1500;  ret.Zone1.LeftTop.y = 10;
  ret.Zone1.RightBottom.x = 1500;  ret.Zone1.RightBottom.y = 3000;
  return ret;
}

void write_eeprom(s_settings ini){

}

void setup() {
  MONITOR_SERIAL.begin(115200);
  MONITOR_SERIAL.println("\nStart...");
  delay(500);
  preferences.begin("settings", false);

  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  delay(500);

  unsigned long previousMillis = millis();

  ini = read_eeprom();
  client.begin();
}

void onMsgCommand( const String &message ){}
void OnLoad(){}

s_state Read(){
  s_state ret = cur_state;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < INTERVAL) return ret;
  
  Position str;
  str = GetCoord();
  Serial.print(str.Target1_X);
  Serial.print(", ");
  Serial.print(str.Target1_Y);
  Serial.print(", ");
  Serial.println(90+str.Target1_angle);
  while (RADAR_SERIAL.available()) RADAR_SERIAL.read();
  previousMillis = currentMillis;

  ret.Zone1_state = 0;
  if(str.Target1_X >= ini.Zone1.LeftTop.x && str.Target1_X <= ini.Zone1.RightBottom.x && str.Target1_Y >= ini.Zone1.LeftTop.y && str.Target1_Y <= ini.Zone1.RightBottom.y ) {
    ret.Zone1_state = 1;
  }
  
  return ret;
}

void report(s_state state, int mode){
  if (mode == 0 || ( mode == 1 && cur_state.Zone1_state != state.Zone1_state ) )  {
    client.Publish("states/Zone1", String(state.Zone1_state));
  }
  client.flag_start = false;
  cur_state = state;
}

void OnCheckState(){}
void onConnection(){}

void loop() {
  client.loop();

  s_state state;
  state = Read();
  report(state, (client.flag_start ? 0 : 1));
}
