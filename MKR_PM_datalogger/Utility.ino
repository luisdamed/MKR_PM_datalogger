////////////// Utility functions //////////////

// Dummy function to wake up Arduino from sleep mode
void alarmEvent0() {
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


void StartSDCard() {
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(4)) {
    Serial.println(F("SD card initialization failed!"));
    while (1) {
      WiFi.setLEDs(0, 255, 255); // light blue
    }
  }
  Serial.println(F("SD card initialization done."));

  myFile = SD.open("log.txt", FILE_WRITE); // create a test file

  if (myFile) { // if the file opened okay, write to it:
    Serial.print(F("Writing to log.txt..."));
    myFile.println("SDS011 Sensor");
    myFile.println("Timestamp\tPM Concentration [μg/m3]\tPM Concentration [μg/m3]\tTemperature [°C]\tRelative humidity %\tAtmospheric pressure in [kPa]");
#ifdef UseAverage
    myFile.print("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\tavgPM2.5\tavgPM10\tavgTemp\tavgRelHum\tavgPress\n");
#else
    myFile.println("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\n");
#endif

    myFile.close();// close the file:
    Serial.println(F("done."));
  } else {
    Serial.println(F("error opening log.txt")); // if the file didn't open, print an error:
    while (1) {
      WiFi.setLEDs(255, 0, 255); // Purple
    }
  }
}

void GetNTPtime () {
  while ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    WiFi.setLEDs(255, 255, 0); // Yellow
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWiFiStatus();

  //Start the internal RTC
  rtc.begin();

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

/* Check the status of the Arduino MKR ENV Shield 
 * and stop if it is not connected or not working*/
void CheckENV_MKR() {
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
  sds.begin();        // start serial communication with the SDS011 sensor
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setQueryReportingMode().toString()); // sets sensor to 'query' reporting mode
#ifdef ContinuousReading
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // set the sensor to operate in continuous mode
#else
  Serial.println(sds.setCustomWorkingPeriod(SensorWorkPeriod / 60).toString()); // set the custom work period of the PM sensor
#endif
}


/* Wake up the sensor from sleep mode and wait 30 seconds
   This is done to obtain a reliable flow rate through the sensor
   and to remove any existing dust deposit that may build up during sleep mode
*/
void WakeUpSDS011() {
#ifndef NoSleep || ContinuousReading
  Serial.println("Wake up SDS011 sensor");
  WorkingStateResult result = sds.wakeup();
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
#endif
#endif
}

//Read from the MKR ENV Shield
void Read_ENV_MKR (float *temp, float *hum, float *pres){

#ifdef DebugMessages
  Serial.println("Getting data from MKR ENV shield");
#endif
  *temp  = ENV.readTemperature(); // Temperature in °C
  *hum   = ENV.readHumidity();    // Relative Humidity %
  *pres   = ENV.readPressure();    // Barometric pressure in kPa
}

void ComputeAvg () { // COmputes the average of the running average of the readings for the number of readings requested by the user
  //Initialize the averages from zero
  if (readIndex == 0) {
    PM25_avg = 0;
    PM10_avg = 0;
    TEMP_avg = 0;
    HUM_avg  = 0;
    PRES_avg = 0;
#ifdef DebugMessages
    Serial.println("Averages initialized to zero");
#endif
  }
  //Update the running average up to the current reading
  PM25_avg += (rawPM25 - PM25_avg) / (readIndex + 1);
  PM10_avg += (rawPM10 - PM10_avg) / (readIndex + 1);
  TEMP_avg += (temperature - TEMP_avg) / (readIndex + 1);
  HUM_avg  += (humidity  - HUM_avg ) / (readIndex + 1);
  PRES_avg += (pressure - PRES_avg) / (readIndex + 1);
}
