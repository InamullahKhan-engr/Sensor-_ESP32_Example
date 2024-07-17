#define SENSOR1_PIN 25  // GPIO pin 4
#define SENSOR2_PIN 26  // GPIO pin 5
#define ANALOG_PIN 32

bool Dir = false;
long Count;
bool New = false;
void IRAM_ATTR sensor1Interrupt() {
  if(digitalRead(SENSOR1_PIN)){
    Count++;
    Dir=true;
  }else{
    Count--;
    Dir=false;
  }
  New = true;
}



void setup() {
  Serial.begin(115200);
  pinMode(SENSOR1_PIN, INPUT_PULLUP);
  pinMode(SENSOR2_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR2_PIN), sensor1Interrupt, RISING);
  pinMode(ANALOG_PIN, INPUT);
}

void loop() {
  
  if (New) {
    New = false;
    Serial.print("Distance: ");
    Serial.print(Count);
    Serial.print("  Direction: ");
    Serial.println(Dir);
  }
  int analogValue = analogRead(ANALOG_PIN);
  Serial.print("Analog value: ");
  Serial.println(analogValue);
  delay(500);
}
