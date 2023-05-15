#include "mqtt_ini.h"

static mqtt_ini* s_mqtt;

mqtt_ini::mqtt_ini( const char* mqttClientName, String _pref_topic, boolean _send_work, uint32_t _WDT_TIMEOUT){
  disable_mqtt = false;

  client = new EspMQTTClient( 
    wifiSsid,
    wifiPassword,
    mqttServerIp,
    mqttUsername,
    mqttPassword,
    mqttClientName,
    mqttServerPort);

  pref_topic = _pref_topic;

  WDT_TIMEOUT = _WDT_TIMEOUT;
  send_work = _send_work;
}

void mqtt_ini::begin(boolean debug){
  flag_Subscribed = false;
  flag_start = true;

  if(debug) client->enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client->enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").

  s_mqtt = this;

  WDT_Enable( );
}

void mqtt_ini::set_disable_mqtt(boolean _disable){
  disable_mqtt = _disable;
}

boolean mqtt_ini::get_disable_mqtt(){
  return disable_mqtt;
}

boolean mqtt_ini::WDT_Disable(){
#ifdef ESP32
  int msg = esp_task_wdt_delete(NULL);
  if(msg == ESP_OK ){
    msg = esp_task_wdt_deinit();
    //if(msg == ESP_OK){
      return true;
    //}
  }
#endif  
  return false;
}

boolean mqtt_ini::WDT_Enable(){
#ifdef ESP32
  if(WDT_TIMEOUT > 0) {
    if(esp_task_wdt_init(WDT_TIMEOUT, true) == ESP_OK){ //enable panic so ESP32 restarts
      if(esp_task_wdt_add(NULL) == ESP_OK) { //add current thread to WDT watch
        return true;
      }
    }
  }
#endif  
return false;
}

boolean mqtt_ini::Subscribe(String subtopic, MessageReceivedCallback messageReceivedCallback, boolean f_ext){
  if(disable_mqtt) return true;

  String sub = "";
  if(!f_ext) {
    sub = String(pref_topic) + "/" + subtopic;  
  } else {
    sub = subtopic;
  }

  return client->subscribe(sub, messageReceivedCallback); 
}

boolean mqtt_ini::Subscribe(String subtopic, MessageReceivedCallbackWithTopic messageReceivedCallback, boolean f_ext){
  if(disable_mqtt) return true;

  String sub = "";
  if(!f_ext) {
    sub = String(pref_topic) + "/" + subtopic;  
  } else {
    sub = subtopic;
  }

   return client->subscribe(sub, messageReceivedCallback); 
}

boolean mqtt_ini::Publish( String topic, String value, boolean w_pref){
  String s_topic = pref_topic;
  if(w_pref){
    s_topic = s_topic + "/" + topic;
  }else{
    s_topic = topic;
  }

  if(disable_mqtt) {
    Serial.println(">>Topic = " + s_topic + ", Message = " + value);
    return true;
  }

  boolean ret = false;

  if(client->isMqttConnected( )) {
    ret = client->publish( s_topic, value );
  }

  return ret;  
}

void mqtt_ini::loop(){
  CheckStateCalled = false;

  client->loop();

  if(client->isMqttConnected() && !disable_mqtt) {
    if((flag_Subscribed && millis( ) - AfterSubscribe > 3000 ) || flag_Runned) {
      if(!flag_Runned) flag_start = true;
      
      if(!flag_Loaded) {
        Publish("load", "1");
        Publish("working", "1");
        flag_Loaded = true;
        OnLoad();
      }
      CheckState( );
      flag_Runned = true;
    }
  } else if(disable_mqtt) {
    if(!flag_Runned) flag_start = true;
    flag_Loaded = flag_Subscribed = true;
    CheckState( );
    flag_Runned = true;
  }

#ifdef ESP32
  if(WDT_TIMEOUT > 0) {
    if(esp_task_wdt_status( NULL ) == ESP_OK){
      esp_task_wdt_reset();
    }
  }
#endif
}

void CheckState( ){
  if (s_mqtt->flag_start) { //первый запуск
    s_mqtt->Publish("command", "");
    s_mqtt->Publish("ip", WiFi.localIP().toString());
    s_mqtt->Publish("mac", WiFi.macAddress());
    s_mqtt->Publish("working", "1");
#ifdef ESP32
    s_mqtt->Publish("WDT", String(s_mqtt->WDT_TIMEOUT));
#endif
  }

  if (millis() - s_mqtt->LastLoaded > IntervalLoaded) {
    if(s_mqtt->send_work) s_mqtt->Publish("working", "1");
    s_mqtt->LastLoaded = millis();
  }

  if(s_mqtt->flag_Runned) {
    OnCheckState( );
    s_mqtt->CheckStateCalled = true;
  }
}

void Msg_command( const String &message ){
  Serial.println("Msg_command: " + message);

  if (message == "reboot") {
    s_mqtt->Publish("working", "0");
    Serial.println("command: reboot");
    ESP.restart();
  } else if(message == "emul_dis_mqtt = on") {
    s_mqtt->set_disable_mqtt(true);
  } else if(message == "emul_dis_mqtt = off") {
    s_mqtt->set_disable_mqtt(false);
  } else if(message == "refresh") {
    Serial.println("command: refresh");
    s_mqtt->flag_start = true;
    CheckState();
  } else if(message == "DIS_WDT") {
    Serial.println("command: DIS_WDT");
    if(s_mqtt->WDT_Disable( )){
      Serial.println("WDT disabled");
    }
  } else if(message == "ENAB_WDT") {
    Serial.println("command: ENAB_WDT");
    if(s_mqtt->WDT_Enable( )){
      Serial.println("WDT enabled");
    }
  } else {
    onMsgCommand(message);
  }
}

void onConnectionEstablished()
{
  String sub = s_mqtt->pref_topic + "/command";
  s_mqtt->client->subscribe(sub, Msg_command);

  onConnection();

  s_mqtt->flag_Subscribed = true;
  s_mqtt->AfterSubscribe = millis( );

}
