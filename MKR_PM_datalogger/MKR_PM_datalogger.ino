/* RTC functions based on  http://arduino.cc/en/Tutorial/WiFiRTC
    The method for determining whether there is a leap year or not
    was proposed by the user econjack here https://forum.arduino.cc/index.php?topic=226313.0

   SDS011 library: https://github.com/lewapek/sds-dust-sensors-arduino-library

   Luis Medina - luis.medina@polito.it
   Release: 21-12-2020
*/
#include <Arduino_MKRENV.h>
#include <SPI.h>
#include <SD.h>
#include <WiFiNINA.h>         //https://github.com/adafruit/WiFiNINA
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include "SdsDustSensor.h"    //https://github.com/lewapek/sds-dust-sensors-arduino-library

const float SensorCycleTime = 1.004; // Cycle time of the SDS011 PM sensor as indicated in the datasheet (in seconds)

/* Define which functionalities you want to use
   By uncommenting the following definitions, you can enable
   different modes of operation of the sensor module.
   Please read the comments to see a brief description of each one
*/

#define WriteToSD;          //  Enable the data-logging functionality
#define UseNTPTime;         //  Using Network Time Protocol to get accurate timestamps
#define UseAverage;         //  Uncomment to enable the use of averaged values
#define NoSleep;            //  Comment this line if you don't want the sensor module to enter low-power operation (sleep)
#define ContinuousReading;  //  Enable the default reading mode of the sensor
//#define DebugMessages;      //  Enable debugging messages

// Wifi connection credentials and reference time zone for NTP
char ssid[] = "my_SSID";        // your network SSID (name)
char pass[] = "testpassword";     // your network password (use for WPA, or use as key for WEP)
const uint8_t GMT = 1; //change this according to your current time zone. Notice the difference between summer and winter time

// PM sensor working parameters
const uint8_t numReadings = 10;    // How many readings the sensor will send after wake up
const float SensorWorkPeriod = 2 * SensorCycleTime;    // Specify how often the sensor should send data (in seconds)
const uint8_t LowPowerTime = 30;         // How many seconds the sensor spends in sleep mode between two consecutive sampling operations

/* -------------------------- No modifications are needed below this line --------------------------------------------------*/


// Initialize  variables
float PM25readings[numReadings];     // the PM2.5 readings during the sampling time
float PM10readings[numReadings];     // the PM10 readings during the sampling time
float TEMPreadings[numReadings];     // the temperature readings during the sampling time
float HUMreadings[numReadings];      // the relative humidity readings during the sampling time
float PRESreadings[numReadings];     // the atmospheric pressure readings during the sampling time
uint8_t readIndex = 0;                   // the index of the current reading
float totalPM10 = 0;                 // the running total for PM10
float totalPM25 = 0;                 // the running total for PM25
float totalTEMP = 0;                 // the running total for TEMP
float totalHUM = 0;                  // the running total for HUM
float totalPRES = 0;                 // the running total for PRES
float reportedPM10 = 0;               // the average PM10
float reportedPM25 = 0;               // the average PM25
float reportedTEMP = 0;               // the average TEMP
float reportedHUM = 0;                // the average HUM
float reportedPRES = 0;               // the average PRES
char timestamp[21];
static bool header = true;


// Create objects
SdsDustSensor sds(Serial1); // define serial port used to communicate with SDS011 sensor
File myFile;                // create object referencing the text file
RTCZero rtc;                // create object referencing the real time clock
int status = WL_IDLE_STATUS;     // define idle status of WIFI connection

void setup() {

  //Allow the board to wake up using an interrupt based on the RTC
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);
  Serial.begin(9600); // establish communication parameters with serial port

  if (! Serial) {
    WiFi.setLEDs(255, 255, 255); // white
    delay(5000);   // Wait a couple seconds while serial monitor is started
  }
  // Check the status of the MKR ENV Shield
  CheckENV_MKR ();

  // Initialize the SDS011 PM sensor
  StartSDS011();

#ifdef WriteToSD
  StartSDCard();
#endif

#ifdef UseNTPTime
  GetNTPtime();
#endif

}


void loop() {

  WiFi.setLEDs(0, 255, 0); // Turn RGB LED Green to mark the start of the loop
  
  if (readIndex == 0) {
    //Wake up the SDS011 sensor and check that it is working correctly
    WakeUpSDS011();
  }

  //Get the current time from RTC
  GetRTCTime();

  WiFi.setLEDs(0, 0, 0); //
 
  PmResult pm = sds.queryPm();

  float rawPM10 = pm.pm10;
  float rawPM25 = pm.pm25;
#ifdef DebugMessages
  Serial.println("Getting data from MKR ENV shield");
#endif
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();

#ifdef UseAverage


  // subtract the last readings:
  if (readIndex == 0) {
    totalPM10 = 0;
    totalPM25 = 0;
    totalTEMP = 0;
    totalHUM  = 0;
    totalPRES = 0;
  }

  // store sensor values:
  PM10readings[readIndex] = rawPM10;
  PM25readings[readIndex] = rawPM25;
  TEMPreadings[readIndex] = temperature;
  HUMreadings[readIndex]  = humidity;
  PRESreadings[readIndex] = pressure;

  // add the readings to the total:
  totalPM10 += PM10readings[readIndex];
  totalPM25 += PM25readings[readIndex];
  totalTEMP += TEMPreadings[readIndex];
  totalHUM  += HUMreadings[readIndex];
  totalPRES += PRESreadings[readIndex];

#endif
  // advance to the next position in the array:
  readIndex += 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {

    // calculate the average:

    reportedPM10 = totalPM10 / numReadings;;               // the average PM10
    reportedPM25 = totalPM25 / numReadings;;               // the average PM25
    reportedTEMP = totalTEMP / numReadings;;               // the average TEMP
    reportedHUM  = totalHUM  / numReadings;                // the average HUM
    reportedPRES = totalPRES / numReadings;                // the average PRES
  }


  if (pm.isOk()) {
    //Write the results in the Serial monitor and the SD card if enabled
    Print_results(timestamp, rawPM25, rawPM10, temperature,  humidity,  pressure,
                    reportedPM25, reportedPM10, reportedTEMP, reportedHUM, reportedPRES);
  }

  else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(pm.statusToString());
    WiFi.setLEDs(255, 0, 0); // RED
  }


#ifdef ContinuousReading
  delay(1000);
#else
  delay(SensorWorkPeriod * 1000);
#endif


  if (readIndex >= numReadings) {
#ifndef NoSleep
    //  Low-power cycle
    Serial.print("Putting SDS011 sensor in sleep mode\r\n");
    WorkingStateResult result = sds.sleep();
    result.isWorking(); // false
    Serial.print("SDS011 sensor is sleeping\r\n");
    //   Put Arduino in low-power state
    Serial.print("Arduino MKR WIFI 1010 is going to low-power consumption mode\r\n");
    WiFi.setLEDs(0, 0, 0); // OFF
    LowPower.sleep(LowPowerTime * 1000);
    alarmEvent0();
    Serial.print("Arduino MKR WIFI 1010 returned to normal operation mode\r\n");

#endif
    readIndex = 0;
  }
}
