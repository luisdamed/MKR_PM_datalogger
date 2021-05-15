/* PM data logger based on Arduino MKR Wifi 1010. Please see Readme.md for details
 *  
 * Tested successfully on Arduino SAMD core version 1.8.7
 *
 * SDS011 library: https://github.com/lewapek/sds-dust-sensors-arduino-library
 * Author: Paweł Kołodziejczyk <lewapek@gmail.com>
 * version 1.5.0
 *
 * WiFiNINA Library for Arduino (Adafruit's fork) https://github.com/adafruit/WiFiNINA
 * Author: Arduino <info@arduino.cc>
 * version 1.3.0
 *
 * RTCZero functions based on  http://arduino.cc/en/Tutorial/WiFiRTC, repo: https://github.com/arduino-libraries/RTCZero
 * Author: Arduino <info@arduino.cc>
 * version 1.6.0
 * The method for detecting leap years is based on a post by user econjack here https://forum.arduino.cc/index.php?topic=226313.0
 * 
 * Arduino Low Power 
 * Author: Arduino <info@arduino.cc>
 * Version 1.2.1
 *
 * Arduino Unique ID library: https://github.com/ricaun/ArduinoUniqueID 
 * Author: Luiz Henrique Cassettari <ricaun@gmail.com>
 * version=1.1.0
 *
 *
 * Luis Medina - luis.medina@polito.it
 * Date: 09-05-2021
 * version 1.1.1 
*/

//#include <Arduino.h>
#include <Arduino_MKRENV.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include <ArduinoUniqueID.h>
#include "SdsDustSensor.h"


/* Uncomment the following definitions to control how the sensor module will work
   Please read the comments to see a brief description of each definition
*/

#define WriteToSD          //  Write the measured values on a text file stored in the micro SD card
#define UseNTPTime         //  Use Network Time Protocol to get accurate timestamps
#define UseAverage         //  Compute the average of the measured data coming from the sensors
//#define NoSleep            //  Prevent the sensor module from entering low-power (sleep) mode
//#define DebugMessages      //  Enable debugging messages

// Wifi connection credentials and reference time zone for NTP
char ssid[] = "my_SSID";        // your network SSID (name)
char pass[] = "testpassword";     // your network password (use for WPA, or use as key for WEP)
const uint8_t GMT = 2; //change this according to your current time zone. Notice the difference between summer and winter time

// PM sensor working parameters
const uint8_t g_numReadings = 5;    // How many readings will be taken from the sensors after the module wakes up
uint16_t g_lowPowerTime = 300;    // How many seconds the datalogger module stays in sleep mode between two consecutive sampling cycles

/* -------------------------- No modifications are needed below this line --------------------------------------------------*/


// Define global variables

/*----------------------Raw values-------------------------*/
//float g_rawPM10 = 0;          // PM10  mass concentration in ug/m3
//float g_rawPM25 = 0;          // PM2.5 mass concentration in ug/m3
float g_temperature = 0;      // Temperature in °C
float g_humidity = 0;         // Relative Humidity %
float g_pressure = 0;         // Barometric pressure in kPa

/*--------------------Averaged values----------------------*/
float g_PM25_avg = 0; // Average PM2.5 [ug/m3]
float g_PM10_avg = 0; // Average PM10  [ug/m3]
float g_TEMP_avg = 0; // Average Temperature [°C]
float g_HUM_avg = 0;  // Average relative humidity [%]
float g_PRES_avg = 0; // Average barometric pressure [kPa]

/*---------------Counting and formatting-------------------*/
uint8_t readIndex = 0;     // current reading index
char timestamp[21];        // to store and print the timestamp strings avoiding the "String" class to reduce heap fragmentation
static bool header = true; // this is changed to false once the header is printed at the top of the data table

SdsDustSensor sds(Serial1); // define serial port used to communicate with SDS011 sensor
File myFile;                // text file
RTCZero rtc;                // real time clock internal to the MKR WiFi 1010
int status = WL_IDLE_STATUS;



void setup() {

  //Initiate serial communication
  Serial.begin(9600);
  
  // Wait a couple seconds while serial monitor is started
  if (! Serial) {
    WiFi.setLEDs(255, 255, 255); // white
    delay(5000);
  }
  // Prints Unique board ID
  Print_board_ID();

  // Check the status of the MKR ENV Shield
  CheckENV_MKR ();

  // Initialize the SDS011 PM sensor
  StartSDS011();

#ifdef WriteToSD
  //Open the SD card, create a txt file and print the header on it
  StartSDCard();
#endif

#ifdef UseNTPTime
  /*Connect to the WIFI network and retrieve the reference time
    Disconnect from WIFI once  the time has been received*/
  GetNTPtime();
#endif
}


void loop() {
  // Turn RGB LED Green to mark the start of the loop
  WiFi.setLEDs(0, 255, 0);

  if (readIndex == 0) {
    //Wake up the SDS011 sensor and check that it is working correctly
    WakeUpSDS011();
  }

  //Get the current time from RTC
  GetRTCTime();

  //Turn off the RGB LED
  WiFi.setLEDs(0, 0, 0); //

  //Query new PM data from the SDS011 sensor
  PmResult pm = sds.queryPm();
  
  //Write the results in the Serial monitor and the SD card if enabled by the user during setup
  if (pm.isOk()) {
    
    // Reads data from the sensors and calculates the average
    // Print results to serial and write to SD card
    Get_data(timestamp, pm.pm25, pm.pm10, g_temperature = ENV.readTemperature(), 
              g_humidity = ENV.readHumidity(), g_pressure = ENV.readPressure());
    
    // Increment the count of readings:
    readIndex += 1;

  }
  else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(pm.statusToString());
    WiFi.setLEDs(255, 0, 0); // Turn the LED RED to indicate the faulty message
  }

  // Wait before restarting the loop
  delay(1000);

  /*If the measuring cycle is complete, go to Low-power state */
  if (readIndex >= g_numReadings) {
    #ifndef NoSleep
      // Set SDS011 to sleep mode (turn off fan and laser)
      PMsensor_to_sleepmode();
  
      // Put Arduino in low-power state
      Module_to_sleep();
      
    #endif
    readIndex = 0;
  }
}
