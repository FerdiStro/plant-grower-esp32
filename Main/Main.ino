#include <esp_adc_cal.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <HTTPClient.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#define WIFI_SSID "LamasParty box"
#define WIFI_PASSWORD "99808417320259711798"
#define MQTT_PUB_MOS "esp32/mos/andi"

#define MQTT_HOST IPAddress(192, 168, 178, 152)
#define MQTT_PORT 1883

//Server
String serverName = "http://192.168.178.86:8081/data/test";

//set to true if Credentials needed
const boolean needCreed = false;
#define MQTT_USER ""
#define MQTT_PW ""

const byte dataIn = 21;

const int pump = 17;
const int v3 = 35;


static const float WET_MAX = 55.00;
static const float WET_MIN = 75.00;
static const float OK_MIN = 75.10;
static const float OK_MAX = 83.00;
static const float DRY_MIN = 83.10; 


AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;
//Interval
const long interval = 10000;


void setup(){  
  pinMode(pump, OUTPUT);
  Serial.begin(115200); 

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(WiFiEvent);
  WiFi.setHostname(String(MQTT_PUB_MOS).c_str());
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);
  pinMode(dataIn, INPUT);
  if(needCreed){
    mqttClient.setCredentials(MQTT_USER, MQTT_PW);
  }

  connectToWifi();

 
} 

void loop() {
   digitalWrite(pump, LOW);
   unsigned long currentMillis = millis();
   if (currentMillis - previousMillis >= interval) {

      //Sensor
      previousMillis = currentMillis;
      int sensorValue = analogRead(32);
      float moisture = map(sensorValue, 0, 4095, 0, 100);
      
      if(isnan(moisture)){
        Serial.println(F("FAILED to read sensor!"));
        return;
      }

      //HTTP
      HTTPClient http;
      String serverPath = serverName + "?mos=" + String(moisture).c_str();
      Serial.println("HTTP-Server Port: "+ serverPath);
      http.begin(serverPath.c_str());      
      int httpResponseCode = http.GET();
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      } 
      
      http.end();
     
  
      
      //MQTT
      uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_MOS, 1, true, String(moisture).c_str());    
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_MOS, packetIdPub1);
      Serial.printf("Message: %.2f \n", moisture);
      
  
   }
}

void pumpAct(){
   digitalWrite(pump, HIGH);

}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(10000);
}
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}
