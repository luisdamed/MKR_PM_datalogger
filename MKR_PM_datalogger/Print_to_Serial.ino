void Print_results(char* timestamp, float rawPM25, float rawPM10, float temperature, float humidity, float pressure,
                     float reportedPM25, float reportedPM10, float reportedTEMP, float reportedHUM, float reportedPRES) {

  // only print header first time
  if (header) {
    Serial.println(F("SDS011 Sensor"));
    Serial.println(F("Timestamp\tPM Concentration [μg/m3]\tPM Concentration [μg/m3]\tTemperature [°C]\tRelative humidity %\tAtmospheric pressure in [kPa]"));
#ifdef UseAverage
    Serial.print(F("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\tavgPM2.5\tavgPM10\tavgTemp\tavgRelHum\tavgPress\r\n"));
#else
    Serial.println(F("Date   Time\tPM2.5\tPM10\tTemp\tRelHum\tPress\r\n"));
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
  char PM25[8];
  char PM10[8];
  char TEMP[8];
  char HUM[8];
  char PRESS[8];

  //Local char arrays to print averaged values avoiding the use of String class
  char avgPM25[8];
  char avgPM10[8];
  char avgTEMP[8];
  char avgHUM[8];
  char avgPRESS[8];

  Serial.print(timestamp); // RTC time to Serial
  Serial.print("\t");
  if (myFile) {
    myFile.print(timestamp); // RTC time to SD
    myFile.print("\t"); // RTC time to SD
  }

  snprintf_P(PM25, sizeof(PM25), PSTR("%.2f\t"), rawPM25);
  Serial.print(PM25); //raw PM2.5 mass concentration [μg/m3] to Serial
  if (myFile) {
    myFile.print(PM25); //raw PM2.5 mass concentration [μg/m3] to SD
  }

  snprintf_P(PM10, sizeof(PM10), PSTR("%.2f\t"), rawPM10);
  Serial.print(PM10); //raw PM10 mass concentration [μg/m3] to Serial
  if (myFile) {
    myFile.print(PM10); //raw PM10 mass concentration [μg/m3] to SD
  }

  snprintf_P(TEMP, sizeof(TEMP), PSTR("%.2f\t"), temperature);
  Serial.print(TEMP); //raw temperature in °C to Serial
  if (myFile) {
    myFile.print(TEMP); //raw temperature in °C to SD
  }

  snprintf_P(HUM, sizeof(HUM), PSTR("%.2f\t"), humidity);
  Serial.print(HUM); //raw relative humidity % to Serial
  if (myFile) {
    myFile.print(HUM); //raw relative humidity % to SD
  }

  snprintf_P(PRESS, sizeof(PRESS), PSTR("%.2f\t"), pressure);
  Serial.print(PRESS); //raw barometric pressure [kPa] to Serial
  if (myFile) {
    myFile.print(PRESS); //raw barometric pressure [kPa] to SD
  }


  //  Serial.print(String(rawPM25) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  //  Serial.print(String(rawPM10) + "\t");//PM10 mass concentration from SDS011 in [μg/m3]
  //  Serial.print(String(temperature) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  //  Serial.print(String(humidity) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  //  Serial.print(String(pressure) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]

  if (readIndex >= numReadings) {

#ifdef UseAverage

    snprintf_P(avgPM25, sizeof(avgPM25), PSTR("%.2f\t"), reportedPM25);
    Serial.print(avgPM25); //avg PM2.5 mass concentration [μg/m3] to Serial
    if (myFile) {
      myFile.print(avgPM25); //avg PM2.5 mass concentration [μg/m3] to SD
    }

    snprintf_P(avgPM10, sizeof(avgPM10), PSTR("%.2f\t"), reportedPM10);
    Serial.print(avgPM10); //avg PM10 mass concentration [μg/m3] to Serial
    if (myFile) {
      myFile.print(avgPM10); //avg PM10 mass concentration [μg/m3] to SD
    }

    snprintf_P(avgTEMP, sizeof(avgTEMP), PSTR("%.2f\t"), reportedTEMP);
    Serial.print(avgTEMP); //avg temperature in °C to Serial
    if (myFile) {
      myFile.print(avgTEMP); //avg temperature in °C to SD
    }

    snprintf_P(avgHUM, sizeof(avgHUM), PSTR("%.2f\t"), reportedHUM);
    Serial.print(avgHUM);   //avg relative humidity % to Serial
    if (myFile) {
      myFile.print(avgHUM); //avg relative humidity % to SD
    }

    snprintf_P(avgPRESS, sizeof(avgPRESS), PSTR("%.2f\t"), reportedPRES);
    Serial.print(avgPRESS); //avg barometric pressure in kPa to Serial
    if (myFile) {
      myFile.print(avgPRESS); //avg barometric pressure in kPa to SD
    }

    //    Serial.print(String(reportedPM25) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    //    Serial.print(String(reportedPM10) + "\t"); //PM10 mass concentration from SDS011 in [μg/m3]
    //    Serial.print(String(reportedTEMP) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    //    Serial.print(String(reportedHUM) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    //    Serial.print(String(reportedPRES) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]

#endif

    Serial.print("\r\n");
  }
  if (myFile) {
    myFile.print("\n");
    // close the file:
    myFile.close();
  }

  Serial.print("\r\n");
}
