const byte led_gpio = 12;
const byte led_gpio2 = 23;


const byte led_in = 2; 

void setup(){
  pinMode(led_gpio, OUTPUT); 
  pinMode(led_in, OUTPUT);
   pinMode(led_gpio2, OUTPUT); 
} 


void loop() {
  digitalWrite(led_in, LOW);
  delay(500);
  digitalWrite(led_in, HIGH);
  
  digitalWrite(led_gpio, HIGH);

 digitalWrite(led_gpio2, HIGH);
}
