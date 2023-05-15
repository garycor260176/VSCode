#ifndef esp32_sim800L_h														//
#define esp32_sim800L_h														//

#include <Arduino.h>

void NewMessage(String msgbody, String msgphone );
void ParseSms_01( String msg );
void USSD_ANSWER( String msg );
void pub_response( String response );

struct stru_datetime {
  byte year = 0;
  byte month = 0;
  byte day = 0; 
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;
  byte timezone = 0;  
};

class esp32_sim800l{
  public:
    esp32_sim800l(HardwareSerial* _SIM800, unsigned long baud, int8_t rxPin, int8_t txPin, String _phones, uint32_t _updatePeriod, boolean _Serialprint);
    void begin();
    void loop();
    void delSMS();
    void sendSMS(String phone, String message);
    void sendSMSinPDU(String phone, String message);
    void sendUSSD(String ussd);

    void Serial_println(String msg);

    bool is_MyPhone(String msgphone);

    String sendATCommand(String cmd, bool waiting);
    boolean IsGrpsConnected();
    stru_datetime GetTime();
    boolean SetAutoTimeSync( );
    String waitResponse();
	
	  bool gprsConnect(const char* apn, const char* user, const char* pwd);
	  bool gprsDisconnect();


  private:
    HardwareSerial* SIM800;
    unsigned long baud;
    int8_t rxPin;
    int8_t txPin;

    String _response = "";
    uint32_t lastUpdate;                                   // Время последнего обновления
    uint32_t updatePeriod;                                  // Проверять каждую минуту
    String phones;
    bool hasmsg;  
    boolean Serialprint = false;

  void parseSMS(String msg);
  boolean isOK(String data);
  unsigned char HexSymbolToChar(char c);
  String UCS2ToString(String s);
  String StringToUCS2(String s);
  unsigned int getCharSize(unsigned char b);
  unsigned int symbolToUInt(const String& bytes);
  String byteToHexString(byte i);
  void getPDUPack(String *phone, String *message, String *result, int *PDUlen);
  String getDAfield(String *phone, bool fullnum);
  bool ParseDateTimeString(String datetime, byte &year, byte &month, byte &day, byte &hours, byte &minutes, byte &seconds, byte &timezone);

};

#endif																			  //