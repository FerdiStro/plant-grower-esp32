#include <esp_adc_cal.h>
#include <driver/adc.h>

const byte debugLed = 13; //status Light on port PIN13
const byte dataIn = 21;

static const float WET_MAX = 55.00;
static const float WET_MIN = 75.00;
static const float OK_MIN = 75.10;
static const float OK_MAX = 83.00;
static const float DRY_MIN = 83.10; 

void setup(){
  Serial.begin(115200); 
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);
  pinMode(debugLed, OUTPUT); 
  pinMode(dataIn, INPUT);
 
} 

void loop() {
   digitalWrite(debugLed, HIGH);
   int sensorValue = analogRead(32);
   float moisture = map(sensorValue, 0, 4095, 0, 100);
   Serial.print("Moisture level: ");
   Serial.println(moisture);
   delay(1000);


 

}
