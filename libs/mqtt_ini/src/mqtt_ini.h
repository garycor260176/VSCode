#ifndef mqtt_ini_h												
#define mqtt_ini_h												

#include "EspMQTTClient.h"

#ifdef ESP32
#include <esp_task_wdt.h>
#endif

#define wifiSsid        "default"
#define wifiPassword    "nthfgtdn"
#define mqttServerIp    "192.168.1.40"
#define mqttUsername    "admin"
#define mqttPassword    "1"
#define mqttServerPort  1883

#define IntervalLoaded 10000 //интервал отправления признака жизни

void onConnection();
void onMsgCommand(const String &message);
void CheckState();
void Msg_command( const String &message );
void OnLoad();
void OnCheckState();

class mqtt_ini{
  public:
    EspMQTTClient* client;
    String pref_topic;
    boolean flag_Subscribed = false;
    uint32_t AfterSubscribe = 0;
    boolean flag_Loaded = false;
    boolean flag_start = false;
    uint32_t LastLoaded = 0;
    boolean flag_Runned = false;
    boolean send_work = true;

    uint32_t WDT_TIMEOUT = 0;

    boolean CheckStateCalled;

    mqtt_ini(
      const char* _mqttClientName,
      String _pref_topic,
      boolean _send_work = true,
      uint32_t _WDT_TIMEOUT= 180
    );

    void begin(boolean debug = false);

    boolean Subscribe( String subtopic, MessageReceivedCallback messageReceivedCallback, boolean f_ext = false);
    boolean Subscribe(String subtopic, MessageReceivedCallbackWithTopic messageReceivedCallback, boolean f_ext = false);
    boolean Publish( String topic, String value, boolean w_pref = true);
    boolean WDT_Disable();
    boolean WDT_Enable();

    void loop();

    void set_disable_mqtt(boolean _disable);
    boolean get_disable_mqtt();

  private: 
    boolean disable_mqtt;
};

#endif																			  //