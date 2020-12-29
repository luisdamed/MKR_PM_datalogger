/* RTC functions based on  http://arduino.cc/en/Tutorial/WiFiRTC
    The method for determining whether there is a leap year or not
    was proposed by the user econjack here https://forum.arduino.cc/index.php?topic=226313.0

   SDS011 library: https://github.com/lewapek/sds-dust-sensors-arduino-library

   Luis Medina - luis.medina@polito.it
   Release: 29-12-2020
*/
#include <Arduino_MKRENV.h>
#include <SPI.h>
#include <SD.h>
#include <WiFiNINA.h>         //https://github.com/adafruit/WiFiNINA
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include "SdsDustSensor.h"    //https://github.com/lewapek/sds-dust-sensors-arduino-library


/* Define which functionalities you want to use
   By uncommenting the following definitions, you can enable
   different modes of operation of the sensor module.
   Please read the comments to see a brief description of each one
*/

#define WriteToSD;          //  Enable the data-logging functionality
#define UseNTPTime;         //  Using Network Time Protocol to get accurate timestamps
#define UseAverage;         //  Uncomment to enable the use of averaged values
//#define NoSleep;            //  Comment this line if you don't want the sensor module to enter low-power operation (sleep)
//#define ContinuousReading;  //  Enable the default reading mode of the sensor
//#define DebugMessages;      //  Enable debugging messages

// Wifi connection credentials and reference time zone for NTP
char ssid[] = "my_SSID";        // your network SSID (name)
char pass[] = "testpassword";     // your network password (use for WPA, or use as key for WEP)
const uint8_t GMT = 1; //change this according to your current time zone. Notice the difference between summer and winter time

// PM sensor working parameters
const uint8_t numReadings = 10;    // How many readings the sensor will send after wake up
const float SensorWorkPeriod = 1;    // Specify how often the sensor should send data (in seconds)
const uint8_t LowPowerTime = 300;         // How many seconds the sensor spends in sleep mode between two consecutive sampling cycles

/* -------------------------- No modifications are needed below this line --------------------------------------------------*/


// Initialize  variables

/*----------------------Raw values-------------------------*/
float rawPM10;          // PM10  mass concentration in ug/m3
float rawPM25;          // PM2.5 mass concentration in ug/m3
float temperature;      // Temperature in °C
float humidity;         // Relative Humidity %
float pressure;         // Barometric pressure in kPa

/*--------------------Averaged values----------------------*/
float PM25_avg; // Average PM2.5 [ug/m3]
float PM10_avg; // Average PM10  [ug/m3]
float TEMP_avg; // Average Temperature [°C]
float HUM_avg;  // Average relative humidity [%]
float PRES_avg; // Average barometric pressure [kPa]

/*---------------Counting and formatting-------------------*/
uint8_t readIndex = 0;     // current reading index
char timestamp[21];        // to store and print the timestamp strings avoiding the "String" class to reduce heap fragmentation
static bool header = true; // this is changed to false once the header is printed at the top of the data table


SdsDustSensor sds(Serial1); // define serial port used to communicate with SDS011 sensor
File myFile;                // text file
RTCZero rtc;                // real time clock internal to the MKR WiFi 1010
int status = WL_IDLE_STATUS;



void setup() {

  //Be prepared to wake up using an interrupt based on the RTC
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);

  //Initiate serial communication
  Serial.begin(9600);

  // Wait a couple seconds while serial monitor is started
  if (! Serial) {
    WiFi.setLEDs(255, 255, 255); // white
    delay(5000);
  }

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
    Disconnect from WIFI once you received the time */
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
  rawPM10 = pm.pm10;  // PM10  mass concentration in ug/m3
  rawPM25 = pm.pm25;  // PM2.5 mass concentration in ug/m3

  //Read from the MKR ENV Shield
#ifdef DebugMessages
  Serial.println("Getting data from MKR ENV shield");
#endif
  temperature = ENV.readTemperature(); // Temperature in °C
  humidity    = ENV.readHumidity();    // Relative Humidity %
  pressure    = ENV.readPressure();    // Barometric pressure in kPa

  //Compute the average of the readings, if requested by the user
#ifdef UseAverage
  ComputeAvg();
#endif

  // Increment the count of readings:
  readIndex += 1;

  //Write the results in the Serial monitor and the SD card if enabled
  if (pm.isOk()) {
    Print_results(timestamp);
  }
  else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(pm.statusToString());
    WiFi.setLEDs(255, 0, 0); // Turn the LED RED to indicate the faulty message
  }


// Wait a certain number of seconds before restaring the loop
#ifdef ContinuousReading
  delay(1000);
#else
  delay(SensorWorkPeriod * 1000);
#endif

  /*If the measuring cycle is complete, go to Low-power state */
  if (readIndex >= numReadings) {
#ifndef NoSleep
    
    // Set SDS011 to sleep mode (turn off fan and laser)
    Serial.print("Putting SDS011 sensor in sleep mode\r\n");
    WorkingStateResult result = sds.sleep();
    result.isWorking(); // false
    Serial.print("SDS011 sensor is sleeping\r\n");
    
    // Put Arduino in low-power state
    Serial.print("Arduino MKR WIFI 1010 is going to low-power consumption mode\r\n");
    WiFi.setLEDs(0, 0, 0); // OFF
    LowPower.sleep(LowPowerTime * 1000); //Wait for the time requested by the user
    alarmEvent0(); //Wake up once the time has passed
    Serial.print("Arduino MKR WIFI 1010 returned to normal operation mode\r\n");

#endif
    readIndex = 0;
  }
}
