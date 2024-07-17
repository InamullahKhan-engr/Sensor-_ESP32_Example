// BF350-3AA Load Cell Module

// Setting the connection pin
#define pinOut 32
// Variables to store the results
int value = 0;
int percentage = 0;

void setup() {
  // Start communication over the serial line
  // at a speed of 9600 baud
  Serial.begin(9600);
}

void loop() {
  // Read the analog value from the set pin
  value = analogRead(pinOut);
  // Convert the read data from the analog range (0-2048)
  // to percentage (0-100)
  percentage = map(value, 0, 2048, 0, 100);
  // Print all information over the serial line
  Serial.print("Read value: ");
  Serial.print(value);
  Serial.print(" | ");
  Serial.print(percentage);
  Serial.println("%");
  // Pause before new measurement
  delay(500);
}
