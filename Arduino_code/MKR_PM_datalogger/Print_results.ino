void Print_results(char* timestamp) {

  // only print header first time
  if (header) {
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

  Serial.print(timestamp); // RTC time to Serial
  Serial.print("\t");
  if (myFile) {
    myFile.print(timestamp); // RTC time to SD
    myFile.print("\t"); // RTC time to SD
  }

  snprintf_P(l_pm25, sizeof(l_pm25), PSTR("%.2f\t"), g_rawPM25);
  Serial.print(l_pm25); //raw PM2.5 mass concentration [μg/m3] to Serial
  if (myFile) {
    myFile.print(l_pm25); //raw PM2.5 mass concentration [μg/m3] to SD
  }

  snprintf_P(l_pm10, sizeof(l_pm10), PSTR("%.2f\t"), g_rawPM10);
  Serial.print(l_pm10); //raw PM10 mass concentration [μg/m3] to Serial
  if (myFile) {
    myFile.print(l_pm10); //raw PM10 mass concentration [μg/m3] to SD
  }

  snprintf_P(l_temp, sizeof(l_temp), PSTR("%.2f\t"), g_temperature);
  Serial.print(l_temp); //raw temperature in °C to Serial
  if (myFile) {
    myFile.print(l_temp); //raw temperature in °C to SD
  }

  snprintf_P(l_hum, sizeof(l_hum), PSTR("%.2f\t"), g_humidity);
  Serial.print(l_hum); //raw relative humidity % to Serial
  if (myFile) {
    myFile.print(l_hum); //raw relative humidity % to SD
  }

  snprintf_P(l_press, sizeof(l_press), PSTR("%.2f\t"), g_pressure);
  Serial.print(l_press); //raw barometric pressure [kPa] to Serial
  if (myFile) {
    myFile.print(l_press); //raw barometric pressure [kPa] to SD
  }

  if (readIndex >= g_numReadings) {

#ifdef UseAverage

    snprintf_P(l_avgPM25, sizeof(l_avgPM25), PSTR("%.2f\t"), g_PM25_avg);
    Serial.print(l_avgPM25); //avg PM2.5 mass concentration [μg/m3] to Serial
    if (myFile) {
      myFile.print(l_avgPM25); //avg PM2.5 mass concentration [μg/m3] to SD
    }

    snprintf_P(l_avgPM10, sizeof(l_avgPM10), PSTR("%.2f\t"), g_PM10_avg);
    Serial.print(l_avgPM10); //avg PM10 mass concentration [μg/m3] to Serial
    if (myFile) {
      myFile.print(l_avgPM10); //avg PM10 mass concentration [μg/m3] to SD
    }

    snprintf_P(l_avgTEMP, sizeof(l_avgTEMP), PSTR("%.2f\t"), g_TEMP_avg);
    Serial.print(l_avgTEMP); //avg temperature in °C to Serial
    if (myFile) {
      myFile.print(l_avgTEMP); //avg temperature in °C to SD
    }

    snprintf_P(l_avgHUM, sizeof(l_avgHUM), PSTR("%.2f\t"), g_HUM_avg);
    Serial.print(l_avgHUM);   //avg relative humidity % to Serial
    if (myFile) {
      myFile.print(l_avgHUM); //avg relative humidity % to SD
    }

    snprintf_P(l_avgPRESS, sizeof(l_avgPRESS), PSTR("%.2f\t"), g_PRES_avg);
    Serial.print(l_avgPRESS); //avg barometric pressure in kPa to Serial
    if (myFile) {
      myFile.print(l_avgPRESS); //avg barometric pressure in kPa to SD
    }

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
