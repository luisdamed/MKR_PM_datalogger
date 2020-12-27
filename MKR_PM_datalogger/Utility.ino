////////////// Utility functions //////////////

  // Dummy function to wake up Arduino from sleep mode
void alarmEvent0() {
}

// Retrieve and display WiFi connection status
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  WiFi.setLEDs(0, 0, 255); // Blue
}


void StartSDS011() {
    sds.begin();        // Initialize serial communication with the SDS011 sensor
//
//    if (!sds.begin()) {
//    Serial.println("SDS011 sensor is not connected");
//    while (1) {
//      WiFi.setLEDs(255, 0, 0); // RED
//      delay (200);
//      WiFi.setLEDs(255, 255, 0); // Yellow
//      delay (200);
//    }
//  }
    
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
//  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode
#ifdef ContinuousReading
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended
#else
  Serial.println(sds.setCustomWorkingPeriod(SensorWorkPeriod / 60).toString()); // sensor sends data every SensorWorkPeriod seconds
#endif
}




void StartSDCard() {
 Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("SD card initialization failed!");
    while (1) {
      WiFi.setLEDs(0, 255, 255); // light blue
    }
  }
  Serial.println("SD card initialization done.");

myFile = SD.open("log.txt", FILE_WRITE); // create a test file

  if (myFile) { // if the file opened okay, write to it:
    Serial.print("Writing to log.txt...");
    myFile.println(F("SDS011 Sensor"));
    myFile.println(F("Timestamp\tPM Concentration [μg/m3]\tPM Concentration [μg/m3]\tTemperature [°C]\tRelative humidity %\tAtmospheric pressure in [kPa]"));
#ifdef UseAverage
    myFile.print(F("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\tavgPM2.5\tavgPM10\tavgTemp\tavgRelHum\tavgPress\n"));
#else
    myFile.println(F("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\n"));
#endif

    myFile.close();// close the file:
    Serial.println("done.");
  } else {
    Serial.println("error opening log.txt"); // if the file didn't open, print an error:
    while (1) {
      WiFi.setLEDs(255, 0, 255); // Purple
    }
  }
}

void GetNTPtime () {
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    WiFi.setLEDs(255, 255, 0); // Yellow
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWiFiStatus();
  //  WiFi.setLEDs(0, 0, 255); // Blue

  rtc.begin();

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1) {
      WiFi.setLEDs(255, 255, 0); // Yellow
    }
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    WiFi.setLEDs(0, 0, 255); // Blue
    delay(1000);
    Serial.println();
  }
  WiFi.disconnect();
  WiFi.end();
  Serial.println("Disconnected from WIFI");
  WiFi.setLEDs(0, 0, 0); // Off
}

void CheckENV_MKR() {
    if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1) {
      WiFi.setLEDs(255, 0, 0); // RED
      delay(200);
      WiFi.setLEDs(0, 0, 0); // Off
      delay(200);
    }
  }
  Serial.println("Initialized communications with MKR ENV shield");
}
