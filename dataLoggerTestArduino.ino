#include <Adafruit_MCP4725.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

Adafruit_MCP4725 dac;

int yellow_LED = 13;
int purple_LED = 12;

int vout = 1241; //reference voltage used for analog inputs (1241 is approximately equal to 1.0V

int A_pin0 = 0; 
int A_pin1 = 1;
int A_pin2 = 2;
int A_pin3 = 3;
int A_pin4 = 4;
int A_pin5 = 5;

int A_pin0_reading;
int A_pin1_reading;
int A_pin2_reading;
int A_pin3_reading;
int A_pin4_reading;
int A_pin5_reading;

bool ECHO_TO_SERIAL = 0;

RTC_PCF8523 rtc;
const int chipSelect = 10;

int sample_rate; //in seconds
int time_logging = 0;
int log_interval = 1000; // mills between entries
uint32_t syncTime = 0; // time of last sync()

int redLEDpin = 1;

File logfile;
char filename[23];

void error(char *str)
{
  //Serial.print("error: ");
  //Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
} 

void setup() {
  //start DAC and set reference voltage to vout of DAC
  dac.begin(0x62);
  dac.setVoltage(vout, 0);
  analogReference(AR_EXTERNAL);

  
  pinMode(yellow_LED, OUTPUT);
  digitalWrite(yellow_LED, HIGH);
  
  // We'll send debugging information via the Serial monitor
  sample_rate = 1;
   
  if (ECHO_TO_SERIAL){
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }  
  }
  
  //The radio uses pin 8 as a Chip Select and the SD card uses pin 10. We must set pin 8 to HIGH and pin 10 to LOW 
  //when we would like to communicate with the SD card, and vice versa when we would like to use the radio.
  pinMode(8, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(10, LOW);

  // initialize the SD card
  //Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(chipSelect, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // connect to RTC
  Wire.begin();  
  if (!rtc.begin()) {
    Serial.println("RTC failed");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //sets RTC to time on computer when compiled
  DateTime now = rtc.now();
  
  //create a new file
  // filenames must be a maximum of 8 characters with 3 character extension
  
  sprintf(filename, "%02d%02d%02d.csv", now.day(), now.month(), now.year());
  
  
  logfile = SD.open(filename, FILE_WRITE); 
  
  
  
  Serial.print("Logging to: ");
  Serial.println(filename);
 
  // If you want to set the aref to something other than 5v
  //analogReference(EXTERNAL);
  logfile.println("hour:minute:second day-month-year,voltage,W/m^2"); 
  digitalWrite(yellow_LED, LOW);
}

 
void loop() {
  pinMode(purple_LED, OUTPUT);
  digitalWrite(purple_LED, HIGH);
  
  delay((log_interval - 1) - (millis() % log_interval));
  logfile = SD.open(filename, FILE_WRITE);
  
  DateTime now = rtc.now();
  //log sensor data 
  if (sample_rate > 1){ //logging rate > 1 s
    time_logging +=1;
    if(time_logging <= sample_rate){
      A_pin0_reading += analogRead(A_pin0); //sum values every second while within capture time
     }
     else {
      logfile.print(",");
      logfile.print(A_pin0_reading/sample_rate);  //the average value over the capture time
      //Serial.print("average reading = ");
      //Serial.print(A_pin0_reading/sample_rate);
      A_pin0_reading = 0;
      time_logging = 0;
     }
    //Serial.print("Analog reading = ");
    //Serial.println(A_pin0_reading);     // the raw analog reading
  }
  else{ //log once per second
    A_pin0_reading = analogRead(A_pin0);
    logfile.print(",");
    logfile.print(A_pin0_reading);
    logfile.print(",");
    logfile.print("values");
      //the average value over the capture time
    //Serial.print("entry");
  }

  //log time
  logfile.print('"');
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print("/");
  logfile.print(now.year(), DEC);
  
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');

  
  
  logfile.println();
  logfile.close();
  //Serial.println();
  
}
    
  
