/* PM data logger based on Arduino MKR Wifi 1010. Please see Readme.md for details

   SDS011 library: https://github.com/lewapek/sds-dust-sensors-arduino-library
   Author: Paweł Kołodziejczyk <lewapek@gmail.com>
   version 1.5.0

   WiFiNINA Library for Arduino (Adafruit's fork) https://github.com/adafruit/WiFiNINA
   Author: Arduino <info@arduino.cc>
   version 1.6.0

   RTCZero functions based on  http://arduino.cc/en/Tutorial/WiFiRTC, repo: https://github.com/arduino-libraries/RTCZero
   Author: Arduino <info@arduino.cc>
   version 1.6.0
   The method for detecting leap years is based on a post by user econjack here https://forum.arduino.cc/index.php?topic=226313.0

   Arduino Unique ID library: https://github.com/ricaun/ArduinoUniqueID 
   Author: Luiz Henrique Cassettari <ricaun@gmail.com>
   version=1.1.0


   Luis Medina - luis.medina@polito.it
   Date: 08-01-2021
   version 1.0.1 
*/

// #include <Arduino.h>
// #include <Arduino_MKRENV.h>
// #include <SPI.h>
// #include <SD.h>
// #include <WiFiNINA.h>
// #include <WiFiUdp.h>
// #include <RTCZero.h>
// #include <ArduinoLowPower.h>
// #include <ArduinoUniqueID.h>
// #include "SdsDustSensor.h"
#include "MKR_PM_datalogger.h"


/* Uncomment the following definitions to control how the sensor module will work
   Please read the comments to see a brief description of each definition
*/

#define WriteToSD;          //  Write the measured values on a text file stored in the micro SD card
#define UseNTPTime;         //  Use Network Time Protocol to get accurate timestamps
#define UseAverage;         //  Compute the average of the measured data coming from the sensors
//#define NoSleep;            //  Prevent the sensor module from entering low-power (sleep) mode
#define ContinuousReading;  //  Configure the PM sensor to measure data as fast as it can (may be unreliable)
//#define DebugMessages;      //  Enable debugging messages

// Wifi connection credentials and reference time zone for NTP
char ssid[] = "my_SSID";        // your network SSID (name)
char pass[] = "testpassword";     // your network password (use for WPA, or use as key for WEP)
const uint8_t GMT = 1; //change this according to your current time zone. Notice the difference between summer and winter time

// PM sensor working parameters
const uint8_t g_numReadings = 10;    // How many readings will be taken from the sensors after the module wakes up
const float g_sensorWorkPeriod = 1;    // Work period of the PM sensor, in seconds
const uint8_t g_lowPowerTime = 300;    // How many seconds the datalogger module stays in sleep mode between two consecutive sampling cycles

/* -------------------------- No modifications are needed below this line --------------------------------------------------*/

MKR_PM_datalogger.MKR_PM_datalogger(g_numReadings, g_sensorWorkPeriod, g_lowPowerTime)



void setup() {

  // Initialize the MKR PM datalogger
  MKR_PM_datalogger.begin(9600);
  


void loop() {

  // Wake up the MKR PM datalogger module
  MKR_PM_datalogger::wakeUp_MKR_PM()

  // //Query new PM data from the SDS011 sensor
  MKR_PM_datalogger.query_MKR_PM();

  //Read from the MKR ENV Shield
  MKR_PM_datalogger.read_ENV_MKR ();

  //Compute the average of the readings, if requested by the user during setup
#ifdef UseAverage
  MKR_PM_datalogger.compute_average();
#endif

  // Increment the count of readings:
  MKR_PM_datalogger.readIndex += 1;

  //Write the results in the Serial monitor and the SD card if enabled by the user during setup
  if (MKR_PM_datalogger.pm.isOk()) {
    MKR_PM_datalogger.printResults();
  }
  else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(MKR_PM_datalogger.pm.statusToString());
    WiFi.setLEDs(255, 0, 0); // Turn the LED RED to indicate the faulty message
  }


  // Wait before restarting the loop
#ifdef ContinuousReading
  delay(1000);
#else
  delay(MKR_PM_datalogger.g_sensorWorkPeriod * 1000);
#endif

  /*If the measuring cycle is complete, go to Low-power state */
  if (MKR_PM_datalogger.readIndex >= MKR_PM_datalogger.g_numReadings) {
    #ifndef NoSleep
      MKR_PM_datalogger.setSleepMode();
    #endif
    MKR_PM_datalogger.readIndex = 0;
  }
}
