/*
  SD Card Interface code for ESP32
  SPI Pins of ESP32 SD card as follows:
  CS    = 5;
  MOSI  = 23;
  MISO  = 19;
  SCK   = 18; 
*/
#include <Arduino.h>
#include "soc/rtc.h"
#include "HX711.h"
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Arduino.h>
#include <EEPROM.h>
int address = 0;
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
#define interval  15
RTC_DS3231 rtc;
const int CS = 5;
#define FILENAME     "/data.csv"
File logFile;
HX711 scale;
bool Mode=false;
float val;
float cfactor;
int isClibrated = 0;
void setup() {
  Serial.begin(115200);    // Set serial baud rate to 9600
  delay(1000);
  
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  EEPROM.begin(sizeof(float) + sizeof(int));
  EEPROM.get(sizeof(float), isClibrated);
  Serial.println(isClibrated);
  if (isClibrated==1) {
    EEPROM.get(address, cfactor);
    Serial.print("Device is calibrated with  C-Factor=");
    Serial.println(cfactor);
    scale.set_scale(cfactor);
  }else{
    Serial.println("Device need calibration");
    bool Mode=true;
  }
  while (!Serial) { ; }  // wait for serial port to connect. Needed for native USB port only
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
rtc.disable32K();
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.disableAlarm(2);
  if (!rtc.setAlarm1(
        rtc.now() + TimeSpan(interval),  // Alarm time: current time + 5 seconds
        DS3231_A1_Minute  // this mode triggers the alarm when the seconds match. See Doxygen for other options
      )) {
    Serial.println("Error, alarm wasn't set!");
  } else {
    Serial.print("Log will be stored every ");
    Serial.print(interval);
    Serial.println(" seconds");
  }
  //ReadFile(FILENAME);
}

void loop() {
  if(Serial.available()){
    char c=Serial.read();
    if(c=='c' || c=='C'){
      Mode=true;
    }
  }
  if(Mode){
    if (scale.is_ready()) {
    scale.set_scale();    
    Serial.println("Remove any weights from the scale.");
    Serial.println("Press 'D' when done");
    while(!WaitForSerial('d'));
    scale.tare();
    Serial.println("Tare done...");
    Serial.print("Place a known weight on the scale...");
    Serial.println("Press 'D' when done");
    while(!WaitForSerial('d'));
    long reading = scale.get_units(10);
    Serial.println("Enter weight you placed. (Kg)");
    Serial.flush();
    while(ReadFloatValue()==-1);
    cfactor = reading/val;
    scale.set_scale(cfactor);
    EEPROM.put(address, cfactor);
    EEPROM.put(sizeof(float), 1);
    EEPROM.commit();
    isClibrated=1;
    Mode=false;
    Serial.print("Calibrate Complete. C Factor is=");Serial.println(cfactor);
  } 
  else {
    Serial.println("HX711 not found.");
  }
  }
  rtc.now();
  if (rtc.alarmFired(1)) {
    LogFile();
    rtc.clearAlarm(1);
    rtc.setAlarm1(
      rtc.now() + TimeSpan(interval),  // Alarm time: current time + interval seconds
      DS3231_A1_Second                          // Alarm mode: every second
    );
  }
 delay(2000);
}


void LogFile(){
  if(isClibrated!=1){
    return;
  }
  if (SD.exists(FILENAME)) {
      logFile = SD.open(FILENAME, FILE_WRITE);
      if (!logFile) {
        Serial.println("Failed to open file for writing");
        return;
      }
      logFile.seek(logFile.size());
    } else {
      logFile = SD.open(FILENAME, FILE_WRITE);
      if (!logFile) {
        Serial.println("Failed to create file for writing");
        return;
      }
      logFile.println("Time, Weight, Depth");
    }
  
  float Weight = getWeight();
  int Depth =  9.174 * Weight + 79.174;
  logFile.print(GetTime());
  logFile.print(",");
  logFile.print(Weight);
  logFile.print(",");
  logFile.println(Depth);
  Serial.print(",");
  Serial.print(Weight);
  Serial.print(",");
  Serial.println(Depth);
  logFile.close();
}
float getWeight(){
    return scale.get_units(10);
}
String GetTime(){
  char Date[24] = "YYYY/MM/DD hh:mm:ss";
  rtc.now().toString(Date);
  Serial.print(Date);
  return String(Date);
}
void ReadFile(const char * path){
  logFile = SD.open(path);
  if (logFile) {
     Serial.printf("Reading file from %s\n", path);
    while (logFile.available()) {
      Serial.write(logFile.read());
    }
    logFile.close(); // close the file:
  } 
  else {
    Serial.printf("Error opening %s\n", path);
  }
}
bool WaitForSerial(char expectedChar) {
  if (Serial.available()) {
    char receivedChar = Serial.read();
    if (receivedChar == expectedChar) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
float ReadFloatValue() {
  if (Serial.available() > 0) {
    float floatValue = Serial.parseFloat();
    if (!isnan(floatValue) && floatValue>0) {
      Serial.println(floatValue);
      val=floatValue;
      return floatValue;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}
