#define FASTLED_ESP8266_RAW_PIN_ORDER
#define CURRENT_LIMIT 5000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

//+SD
#include <FastLED.h>
#include <SD.h>
#include <FS.h>
#include <SPI.h>

#define SD_MISO     19
#define SD_MOSI     23
#define SD_SCLK     18
#define SD_CS       5
//-SD

#define LED_BUILTIN 2

#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN            4  //вывод на матрицу
#define NUMPIXELS      50  //Количество светодиодов

//+SD
#define LED_PIN 4    // вывод на матрицу для SD
#define CHIPSET WS2812B  // your LED chip type
#define CMD_NEW_DATA 1

unsigned char x = 16; // количество светодиодов по X
unsigned char y = NUMPIXELS/x; // количество светодиодов по Y
bool SdCard=true;
File fxdata;
CRGB leds[NUMPIXELS];
//-SD

Adafruit_NeoPixel pixels= Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid     = "default";  //Имя вашей сети Wi-Fi
const char* password = "nthfgtdn"; //пароль вашей сети Wi-Fi
IPAddress ip(192, 168, 1, 145	); //IP адрес гирлянды
IPAddress gateway(192, 168, 1, 1); //IP адрес Wi-Fi роутера
IPAddress subnet(255, 255, 255, 0);
unsigned int localPort = 65506;      // local port to listen for UDP packets
const int PACKET_SIZE = NUMPIXELS*3+7;
byte packetBuffer[PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

typedef struct
{
      byte g;
      byte r;
      byte b;
} colorpin;

colorpin led;
int led_index = 0;

void Sortled ();
int serialGlediator ();

void WiFiEvent(WiFiEvent_t event)
{
      switch (event)
      {
            case SYSTEM_EVENT_STA_GOT_IP:
                  pixels.begin();
                  break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                  break;
/*
            case WIFI_EVENT_STAMODE_GOT_IP:
                  pixels.begin();
                  break;
            case WIFI_EVENT_STAMODE_DISCONNECTED:
                  break;
*/
      }
}

SPIClass sdSPI(VSPI);

boolean InitSDCard(){
// Mount file system
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI))
  {
    // if(!SD.begin()){
    Serial.println("Mounting card mount failed");
    return false;
  }
  uint8_t cardType = SD.cardType();
 
  if (cardType == CARD_NONE)
  {
    Serial.println("Unconnected Memory Card");
    return false;
  }
  else if (cardType == CARD_MMC)
  {
    Serial.println("Mount MMC card");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("Mounting the SDSC Card");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("Mount SDHC Card");
  }
  else
  {
    Serial.println("Mount unknown memory card");
  }
  return true;
}

void setup()
{
//Если есть SD карта

  FastLED.addLeds<CHIPSET, LED_PIN, GRB>(leds, NUMPIXELS); //see doc for different LED strips
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
//  Serial.begin(BAUD_RATE); // when using Glediator via usb
  Serial.begin(115200);

  for(int y = 0 ; y < NUMPIXELS ; y++)
  {
    leds[y] = CRGB::Grey; // set all leds to black during setup
    //Serial.println(y + "Black");
  }
  FastLED.show();

  SdCard = false;
  if(InitSDCard()) {
    // test file open
    fxdata = SD.open("01.out");  // read only
    if (fxdata){
      Serial.println("file open ok test");
      fxdata.close();
      SdCard = true;
    }
  }
  
  if(!SdCard) {
     pinMode(LED_BUILTIN, OUTPUT);
     //Serial.begin(115200);
      WiFi.disconnect(true);
      delay(1000);
      WiFi.onEvent(WiFiEvent);
      WiFi.config(ip, gateway, subnet);
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED)
      {
            delay(300);
            //  Serial.print("x");
            digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
            delay(100);                       // wait for a second
            digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
            delay(100);
      }

    //Serial.println("");
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
      digitalWrite(LED_BUILTIN, LOW);

    //Serial.println("Starting UDP");
      udp.begin(localPort);
    //Serial.print("Local port: ");
    //Serial.println(udp.localPort());
  }
}

void loop()
{
  return;
  // если есть SD карта
  if(SdCard){
    
    fxdata = SD.open("01.out");  // read only
    if (fxdata){
      Serial.println("file open ok - loop");
    }
  
    while (fxdata.available()){
      Sortled();
      FastLED.show();
      delay(20); // set the speed of the animation. 20 is appx ~ 500k bauds
    }
  
    // close the file in order to prevent hanging IO or similar throughout time
    fxdata.close();
    for(int y = 0 ; y < NUMPIXELS ; y++){
      leds[y] = CRGB::Black; // set all leds to black during setup
    }
    FastLED.show();
  }
  else{
    int cb = udp.parsePacket();
      if (!cb){
          //    Serial.setDebugOutput(true);
      }
      else{
          // We've received a packet, read the data from it
          udp.read(packetBuffer, PACKET_SIZE); // read the packet into the buffer
          if (cb >= 6 && packetBuffer[0] == 0x9C){
                // header identifier (packet start)
                byte blocktype = packetBuffer[1]; // block type (0xDA)
                unsigned int framelength = ((unsigned int)packetBuffer[2] << 8) | (unsigned int)packetBuffer[3]; // frame length (0x0069) = 105 leds
                    //Serial.print("Frame.");
                    //Serial.println(framelength); // chan/block
                byte packagenum = packetBuffer[4];   // packet number 0-255 0x00 = no frame split (0x01)
                byte numpackages = packetBuffer[5];   // total packets 1-255 (0x01)

                if (blocktype == 0xDA){
                      // data frame ...
                              ////Serial.println("command");

                      int packetindex;

                      if (cb >= framelength + 7 && packetBuffer[6 + framelength] == 0x36){
                            // header end (packet stop)
                            //Serial.println("s:");
                            int i = 0;
                            packetindex = 6;
                            if (packagenum == 1){
                                  led_index = 0;
                            }
                            while (packetindex < (framelength + 6)){
                                  led.g = ((int)packetBuffer[packetindex]);
                                  led.r = ((int)packetBuffer[packetindex + 1]);
                                  led.b = ((int)packetBuffer[packetindex + 2]);
                                  pixels.setPixelColor(led_index, led.r, led.g, led.b);
                                  led_index++;
                                  //Serial.println(led_index);

                                  packetindex += 3;
                            }
                      }
                        //Serial.print(packagenum);
                        //Serial.print("/");
                        //Serial.println(numpackages);

                }
                if ((packagenum == numpackages) && (led_index == NUMPIXELS)){
                  pixels.show();
                  led_index == 0;
                }
          }
      }
  }
}
  void Sortled () {
    CRGB templeds[NUMPIXELS];
  const uint16_t XYTable[] = {
    15,  16,  47,  48,  79,  80, 111, 112, 143, 144, 175, 176, 207, 208, 239, 240,
    14,  17,  46,  49,  78,  81, 110, 113, 142, 145, 174, 177, 206, 209, 238, 241,
    13,  18,  45,  50,  77,  82, 109, 114, 141, 146, 173, 178, 205, 210, 237, 242,
    12,  19,  44,  51,  76,  83, 108, 115, 140, 147, 172, 179, 204, 211, 236, 243,
    11,  20,  43,  52,  75,  84, 107, 116, 139, 148, 171, 180, 203, 212, 235, 244,
    10,  21,  42,  53,  74,  85, 106, 117, 138, 149, 170, 181, 202, 213, 234, 245,
     9,  22,  41,  54,  73,  86, 105, 118, 137, 150, 169, 182, 201, 214, 233, 246,
     8,  23,  40,  55,  72,  87, 104, 119, 136, 151, 168, 183, 200, 215, 232, 247,
     7,  24,  39,  56,  71,  88, 103, 120, 135, 152, 167, 184, 199, 216, 231, 248,
     6,  25,  38,  57,  70,  89, 102, 121, 134, 153, 166, 185, 198, 217, 230, 249,
     5,  26,  37,  58,  69,  90, 101, 122, 133, 154, 165, 186, 197, 218, 229, 250,
     4,  27,  36,  59,  68,  91, 100, 123, 132, 155, 164, 187, 196, 219, 228, 251,
     3,  28,  35,  60,  67,  92,  99, 124, 131, 156, 163, 188, 195, 220, 227, 252,
     2,  29,  34,  61,  66,  93,  98, 125, 130, 157, 162, 189, 194, 221, 226, 253,
     1,  30,  33,  62,  65,  94,  97, 126, 129, 158, 161, 190, 193, 222, 225, 254,
     0,  31,  32,  63,  64,  95,  96, 127, 128, 159, 160, 191, 192, 223, 224, 255
  };
  
    fxdata.readBytes((char*)templeds, NUMPIXELS*3);
    for (int k=0; k < NUMPIXELS; k++){
      leds[XYTable[k]] = templeds[k];
    }
  }
int serialGlediator (){
  while (!Serial.available()) {}
  return Serial.read();
}