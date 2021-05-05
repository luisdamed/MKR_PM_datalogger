#include "MKR_PM_datalogger.h"

MKR_PM_datalogger::MKR_PM_datalogger(int numReadings = 10, int sensorWorkPeriod = 1, int lowPowerTime = 300){

    MKR_PM_datalogger.g_numReadings = numReadings;
    MKR_PM_datalogger.g_sensorWorkPeriod = sensorWorkPeriod;
    MKR_PM_datalogger.g_lowPowerTime = lowPowerTime;
}



// Prints Unique board ID
void Print_board_ID() {
  Serial.print(F("Arduino board unique ID: "));
  
  for (size_t i = 0; i < UniqueIDsize; i++)
  {
    if (UniqueID[i] < 0x10)
      Serial.print("0");
    Serial.print(UniqueID[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
}



/* Check the status of the Arduino MKR ENV Shield
   and stop if it is not connected or not working*/
void CheckENV_MKR(){
  if (!ENV.begin()) {
    Serial.println(F("Failed to initialize MKR ENV shield!"));
    while (1) {
      WiFi.setLEDs(255, 0, 0); // RED
      delay(200);
      WiFi.setLEDs(0, 0, 0); // Off
      delay(200);
    }
  }
  Serial.println(F("Initialized communications with MKR ENV shield"));
}



/* Setup the SDS011 PM sensor */
void StartSDS011() {
  MKR_PM_datalogger.sds.begin();        // start serial communication with the SDS011 sensor
  Serial.println(MKR_PM_datalogger.sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(MKR_PM_datalogger.sds.setQueryReportingMode().toString()); // sets sensor to 'query' reporting mode
#ifdef ContinuousReading
  Serial.println(MKR_PM_datalogger.sds.setContinuousWorkingPeriod().toString()); // set the sensor to operate in continuous mode
#else
  Serial.println(MKR_PM_datalogger.sds.setCustomWorkingPeriod(MKR_PM_datalogger.g_sensorWorkPeriod / 60).toString()); // set the custom work period of the PM sensor
#endif
}




// Start the SD card and create a text file
void StartSDCard() {
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(4)) {
    Serial.println(F("SD card initialization failed!"));
    while (1) {
      WiFi.setLEDs(0, 255, 255); // light blue
    }
  }
  Serial.println(F("SD card initialization done."));

  // Create a test file
  MKR_PM_datalogger.myFile = SD.open("log.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (MKR_PM_datalogger.myFile) {
    Serial.print(F("Writing to log.txt..."));

    // Write the unique ID of the Arduino board
    MKR_PM_datalogger.myFile.print("Board_ID: ");
    for (size_t i = 0; i < UniqueIDsize; i++)
    {
      if (UniqueID[i] < 0x10)
        MKR_PM_datalogger.myFile.print("0");
      MKR_PM_datalogger.myFile.print(UniqueID[i], HEX);
      MKR_PM_datalogger.myFile.print(" ");
    }
    MKR_PM_datalogger.myFile.println();

    MKR_PM_datalogger.myFile.println("Timestamp\tPM Concentration [μg/m3]\tPM Concentration [μg/m3]\tTemperature [°C]\tRelative humidity %\tAtmospheric pressure in [kPa]");
#ifdef UseAverage
    MKR_PM_datalogger.myFile.print("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\tavgPM2.5\tavgPM10\tavgTemp\tavgRelHum\tavgPress\n");
#else
    MKR_PM_datalogger.myFile.println("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\n");
#endif

    // Close the file:
    MKR_PM_datalogger.myFile.close();
    Serial.println(F("done."));
  } else {
    Serial.println(F("error opening log.txt")); // if the file didn't open, print an error:
    while (1) {
      WiFi.setLEDs(255, 0, 255); // Purple
    }
  }
}

// Get time reference from NTP
void GetNTPtime () {
  while ( MKR_PM_datalogger.status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    WiFi.setLEDs(255, 255, 0); // Yellow
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    MKR_PM_datalogger.status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWiFiStatus();

  //Start the internal RTC
  MKR_PM_datalogger.rtc.begin();

  unsigned long epoch;
  uint8_t numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print(F("NTP unreachable!!"));
    while (1) {
      WiFi.setLEDs(255, 255, 0); // Yellow
    }
  }
  else {
    Serial.print(F("Epoch received: "));
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    WiFi.setLEDs(0, 0, 255); // Blue
    delay(1000);
    Serial.println();
  }
  WiFi.disconnect();
  WiFi.end();
  Serial.println(F("Disconnected from WIFI"));
  WiFi.setLEDs(0, 0, 0); // Off
}


// Retrieve and display WiFi connection status
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
  WiFi.setLEDs(0, 0, 255); // Blue
}




/* Wake up the sensor from sleep mode and wait 30 seconds
   This is done to obtain a reliable flow rate through the sensor
   and to remove any existing dust deposit that may build up during sleep mode
*/
void WakeUpSDS011() {
#ifndef NoSleep
  Serial.println("Wake up SDS011 sensor");
  WorkingStateResult result = MKR_PM_datalogger.sds.wakeup();
  result.isWorking(); // true

  /*Warn the user if the sensor is not connected
    The program will stop and the RGB light will blink red and yellow rapidly
  */
    if (result.isWorking() == false) {
      Serial.println(F("SDS011 sensor is not connected"));
      while (1) {
        WiFi.setLEDs(255, 0, 0); // RED
        delay (200);
        WiFi.setLEDs(255, 255, 0); // Yellow
        delay (200);
      }
  }

  Serial.println("Waiting 30 seconds to have reliable readings from SDS011 sensor\n\rafter wake up from power off or sleep mode");
  delay(30000); //Wait 30 seconds to obtain reliable readings from the sensor
#else
    #ifdef DebugMessages
      Serial.println("Start of sensors sampling interval");
    #endif // Debug messages statement
#endif // NoSleep statement
}






void MKR_PM_datalogger:compute_average(){ 
    // COmputes the average of the running average of the readings for the number of readings requested by the user
    
    //Initialize the averages from zero
    if (MKR_PM_datalogger.readIndex == 0) {
        MKR_PM_datalogger.g_PM25_avg = 0;
        MKR_PM_datalogger.g_PM10_avg = 0;
        MKR_PM_datalogger.g_TEMP_avg = 0;
        MKR_PM_datalogger.g_HUM_avg  = 0;
        MKR_PM_datalogger.g_PRES_avg = 0;
    #ifdef DebugMessages
        Serial.println("Averages initialized to zero");
    #endif
    }
    //Update the running average up to the current reading
    MKR_PM_datalogger.g_PM25_avg += (MKR_PM_datalogger.g_rawPM25 - MKR_PM_datalogger.g_PM25_avg) / (MKR_PM_datalogger.readIndex + 1);
    MKR_PM_datalogger.g_PM10_avg += (MKR_PM_datalogger.g_rawPM10 - MKR_PM_datalogger.g_PM10_avg) / (MKR_PM_datalogger.readIndex + 1);
    MKR_PM_datalogger.g_TEMP_avg += (MKR_PM_datalogger.g_temperature - MKR_PM_datalogger.g_TEMP_avg) / (MKR_PM_datalogger.readIndex + 1);
    MKR_PM_datalogger.g_HUM_avg  += (MKR_PM_datalogger.g_humidity  - MKR_PM_datalogger.g_HUM_avg ) / (MKR_PM_datalogger.readIndex + 1);
    MKR_PM_datalogger.g_PRES_avg += (MKR_PM_datalogger.g_pressure - MKR_PM_datalogger.g_PRES_avg) / (MKR_PM_datalogger.readIndex + 1);
}





// Set SDS011 to sleep mode (turn off fan and laser)
void PMsensor_to_sleepmode() {
  Serial.print("Putting SDS011 sensor in sleep mode\r\n");
  WorkingStateResult result = MKR_PM_datalogger.sds.sleep();
  result.isWorking(); // false
  Serial.print("SDS011 sensor is sleeping\r\n");
}

// Put Arduino in low-power state
void Module_to_sleep() {
  Serial.print("Arduino MKR WIFI 1010 is going to low-power consumption mode\r\n");
  WiFi.setLEDs(0, 0, 0); // OFF
  LowPower.sleep(MKR_PM_datalogger.g_lowPowerTime * 1000); //Wait for the time requested by the user
  alarmEvent0(); //Wake up once the time has passed
  Serial.print("Arduino MKR WIFI 1010 returned to normal operation mode\r\n");
}



void MKR_PM_datalogger::begin(int baudRate = 9600){

    // Dummy function to wake up Arduino from sleep mode
    void alarmEvent0() {}

    //Define interrupt event to wake up from low power mode
    LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);

    //Initiate serial communication
    Serial.begin(baudRate);
  
    // Wait a couple seconds while serial monitor is started
    if (! Serial) {
        WiFi.setLEDs(255, 255, 255); // turn on LED: white color
        delay(5000);
    }
    

    Print_board_ID();
    CheckENV_MKR();
    StartSDS011();
    
    #ifdef WriteToSD
    // Open the SD card, create a txt file and print the header on it
    StartSDCard();
    #endif

    #ifdef UseNTPTime
    // Connect to the WIFI network and retrieve the reference time
    // Disconnect from WIFI once  the time has been received
    GetNTPtime();
    #endif
}

void MKR_PM_datalogger::wakeUp_MKR_PM(){
    // Turn RGB LED Green to mark the start of the loop
    WiFi.setLEDs(0, 255, 0);

    if (MKR_PM_datalogger.readIndex == 0) {
        //Wake up the SDS011 sensor and check that it is working correctly
        WakeUpSDS011();
    }

    //Get the current time from RTC
    GetRTCTime();

    //Turn off the RGB LED
    WiFi.setLEDs(0, 0, 0); //

}

void MKR_PM_datalogger::query_MKR_PM(){

  //Query new PM data from the SDS011 sensor
  MKR_PM_datalogger.pm = MKR_PM_datalogger.sds.queryPm();
  MKR_PM_datalogger.g_rawPM10 = pm.pm10;  // PM10  mass concentration in ug/m3
  MKR_PM_datalogger.g_rawPM25 = pm.pm25;  // PM2.5 mass concentration in ug/m3

}

//Read from the MKR ENV Shield
void MKR_PM_datalogger::read_ENV_MKR(){

#ifdef DebugMessages
  Serial.println("Getting data from MKR ENV shield");
#endif
  MKR_PM_datalogger.g_temperature  = ENV.readTemperature(); // Temperature in °C
  MKR_PM_datalogger.g_humidity     = ENV.readHumidity();    // Relative Humidity %
  MKR_PM_datalogger.g_pressure     = ENV.readPressure();    // Barometric pressure in kPa
}



int leapyear(uint16_t *testyear)
{
  if ((*testyear) % 4 == 0 && (*testyear) % 100 != 0 || (*testyear) % 400 == 0)
    return 1;  // This is a leap year
  else
    return 0;
}

void calendartime (uint8_t *l_hour, uint8_t *l_day, uint8_t *l_month, uint16_t *l_year, uint8_t *l_daysInFebruary)
{
  //Workaround for RTC Zero library ver 1.6.0 bug
  *l_hour = *l_hour - 24;
  *l_day = *l_day + 1;

  //Move from one month to another or change year
  if (*l_month == 1 || *l_month == 3 || *l_month == 5 || *l_month == 7 || *l_month == 8 || *l_month == 10) {
    //If the month has 31 days, change month when the day is already 31 (except for new year)
    if (*l_day > 31) {
      *l_day = 1;
      *l_month = *l_month + 1;
    }
  }
  else if  (*l_month == 4 || *l_month == 6 || *l_month == 9 || *l_month == 11) {
    //If the month has 30 days, change month when the day is already 30
    if (*l_day > 30) {
      *l_day = 1;
      *l_month = *l_month + 1;
    }
  }
  else if (*l_month == 2) {
    //Move from February to March
    if (*l_day > *l_daysInFebruary) {
      *l_day = 1;
      *l_month = *l_month + 1;
    }
  }
  else if (*l_month == 12) {
    //Move from December to January of the next year
    if (*l_day > 31) {
      *l_day = 1;
      *l_month = 1;
      *l_year = *l_year + 1;
    }
  }
}



void MKR_PM_datalogger::GetRTCTime() {

#ifdef DebugMessages
  Serial.println("\n\nGetting current time from RTC");
#endif

  uint8_t hour = (MKR_PM_datalogger.rtc.getHours() + GMT);
  uint8_t mins = (MKR_PM_datalogger.rtc.getMinutes());
  uint8_t sec = (MKR_PM_datalogger.rtc.getSeconds());
  uint8_t day = (MKR_PM_datalogger.rtc.getDay());
  uint8_t month = (MKR_PM_datalogger.rtc.getMonth());
  uint16_t year = (MKR_PM_datalogger.rtc.getYear());
  uint8_t daysInFebruary = 28 + leapyear(&year); 
  if (hour >= 24) {
    calendartime (&hour, &day, &month, &year, &daysInFebruary);
  }
  snprintf_P(MKR_PM_datalogger.timestamp, sizeof(MKR_PM_datalogger.timestamp), PSTR("%02u/%02u/%u %02u:%02u:%02u"), day, month, year, hour, mins, sec);
}



void Print_results() {

  // only print header first time
  if (MKR_PM_datalogger.header) {
    Serial.println(F("SDS011 Sensor"));
    Serial.println(F("Timestamp\tPM Concentration [μg/m3]\tPM Concentration [μg/m3]\tTemperature [°C]\tRelative humidity %\tAtmospheric pressure in [kPa]"));
#ifdef UseAverage
    Serial.print(F("Date     Time   \tPM2.5\tPM10\tTemp\tRelHum\tPress\tavgPM2.5\tavgPM10\tavgTemp\tavgRelHum\tavgPress\r\n"));
#else
    Serial.println(F("Date     Time   \tPM2.5\tPM10\tTemp\tRelHum\tPress\r\n"));
#endif
    header = false;
  }

#ifdef WriteToSD
  //Open the log file
  myFile = SD.open("log.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
#ifdef DebugMessages
    Serial.print("Writing to log.txt...\n");
#endif
  }
#endif

  //Local char arrays to print raw values avoiding the use of String class
  char l_pm25[8];
  char l_pm10[8];
  char l_temp[8];
  char l_hum[8];
  char l_press[8];

  //Local char arrays to print averaged values avoiding the use of String class
  char l_avgPM25[8];
  char l_avgPM10[8];
  char l_avgTEMP[8];
  char l_avgHUM[8];
  char l_avgPRESS[8];

  Serial.print(MKR_PM_datalogger.timestamp); // RTC time to Serial
  Serial.print("\t");
  if (MKR_PM_datalogger.myFile) {
    myFile.print(MKR_PM_datalogger.timestamp); // RTC time to SD
    myFile.print("\t"); // RTC time to SD
  }

  snprintf_P(l_pm25, sizeof(l_pm25), PSTR("%.2f\t"), MKR_PM_datalogger.g_rawPM25);
  Serial.print(l_pm25); //raw PM2.5 mass concentration [μg/m3] to Serial
  if (MKR_PM_datalogger.myFile) {
    myFile.print(l_pm25); //raw PM2.5 mass concentration [μg/m3] to SD
  }

  snprintf_P(l_pm10, sizeof(l_pm10), PSTR("%.2f\t"), MKR_PM_datalogger.g_rawPM10);
  Serial.print(l_pm10); //raw PM10 mass concentration [μg/m3] to Serial
  if (MKR_PM_datalogger.myFile) {
    myFile.print(l_pm10); //raw PM10 mass concentration [μg/m3] to SD
  }

  snprintf_P(l_temp, sizeof(l_temp), PSTR("%.2f\t"), MKR_PM_datalogger.g_temperature);
  Serial.print(l_temp); //raw temperature in °C to Serial
  if (MKR_PM_datalogger.myFile) {
    myFile.print(l_temp); //raw temperature in °C to SD
  }

  snprintf_P(l_hum, sizeof(l_hum), PSTR("%.2f\t"), MKR_PM_datalogger-g_humidity);
  Serial.print(l_hum); //raw relative humidity % to Serial
  if (MKR_PM_datalogger.myFile) {
    myFile.print(l_hum); //raw relative humidity % to SD
  }

  snprintf_P(l_press, sizeof(l_press), PSTR("%.2f\t"), MKR_PM_datalogger.g_pressure);
  Serial.print(l_press); //raw barometric pressure [kPa] to Serial
  if (MKR_PM_datalogger.myFile) {
    myFile.print(l_press); //raw barometric pressure [kPa] to SD
  }

  if (MKR_PM_datalogger.readIndex >= MKR_PM_datalogger.g_numReadings) {

#ifdef UseAverage

    snprintf_P(l_avgPM25, sizeof(l_avgPM25), PSTR("%.2f\t"), MKR_PM_datalogger.g_PM25_avg);
    Serial.print(l_avgPM25); //avg PM2.5 mass concentration [μg/m3] to Serial
    if (MKR_PM_datalogger.myFile) {
      myFile.print(l_avgPM25); //avg PM2.5 mass concentration [μg/m3] to SD
    }

    snprintf_P(l_avgPM10, sizeof(l_avgPM10), PSTR("%.2f\t"), MKR_PM_datalogger.g_PM10_avg);
    Serial.print(l_avgPM10); //avg PM10 mass concentration [μg/m3] to Serial
    if (MKR_PM_datalogger.myFile) {
      myFile.print(l_avgPM10); //avg PM10 mass concentration [μg/m3] to SD
    }

    snprintf_P(l_avgTEMP, sizeof(l_avgTEMP), PSTR("%.2f\t"), MKR_PM_datalogger.g_TEMP_avg);
    Serial.print(l_avgTEMP); //avg temperature in °C to Serial
    if (MKR_PM_datalogger.myFile) {
      myFile.print(l_avgTEMP); //avg temperature in °C to SD
    }

    snprintf_P(l_avgHUM, sizeof(l_avgHUM), PSTR("%.2f\t"), MKR_PM_datalogger.g_HUM_avg);
    Serial.print(l_avgHUM);   //avg relative humidity % to Serial
    if (MKR_PM_datalogger.myFile) {
      myFile.print(l_avgHUM); //avg relative humidity % to SD
    }

    snprintf_P(l_avgPRESS, sizeof(l_avgPRESS), PSTR("%.2f\t"), MKR_PM_datalogger.g_PRES_avg);
    Serial.print(l_avgPRESS); //avg barometric pressure in kPa to Serial
    if (MKR_PM_datalogger.myFile) {
      myFile.print(l_avgPRESS); //avg barometric pressure in kPa to SD
    }

#endif

    Serial.print("\r\n");
  }
  if (MKR_PM_datalogger.myFile) {
    MKR_PM_datalogger.myFile.print("\n");
    // close the file:
    MKR_PM_datalogger.myFile.close();
  }

  Serial.print("\r\n");
}




void MKR_PM_datalogger::printResults(){

if (MKR_PM_datalogger.pm.isOk()) {
    
    Print_results(timestamp);

  }
  else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(MKR_PM_datalogger.pm.statusToString());
    WiFi.setLEDs(255, 0, 0); // Turn the LED RED to indicate the faulty message
  }


}


void MKR_PM_datalogger::setSleepMode(){

    // Set SDS011 to sleep mode (turn off fan and laser)
    Serial.print("Putting SDS011 sensor in sleep mode\r\n");
    WorkingStateResult result = MKR_PM_datalogger.sds.sleep();
    result.isWorking(); // false
    Serial.print("SDS011 sensor is sleeping\r\n");

    // Put Arduino in low-power state
    Serial.print("Arduino MKR WIFI 1010 is going to low-power consumption mode\r\n");
    WiFi.setLEDs(0, 0, 0); // OFF
    LowPower.sleep(MKR_PM_datalogger.g_lowPowerTime * 1000); //Wait for the time requested by the user
    alarmEvent0(); //Wake up once the time has passed
    Serial.print("Arduino MKR WIFI 1010 returned to normal operation mode\r\n");

}




