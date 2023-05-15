#include <cl_ntpclient.h>

const char* ntp_server = "192.168.1.40";

const char* ssid       = "default";   //Replace with your own SSID
const char* password   = "nthfgtdn";  //Replace with your own password


cl_ntpclient ntp(ntp_server);

void setup()
{
  Serial.begin(115200);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTED to WIFI");

  ntp.begin( );

  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
}

void loop()
{
  delay(10000);

  tm timeinfo = ntp.get_Time( );

  Serial.println("День недели: " + String(timeinfo.tm_wday) + " " + "Время: " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec));
}
