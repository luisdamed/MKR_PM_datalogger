void Print_to_Serial(String timestamp, float rawPM25, float rawPM10, float temperature, float humidity, float pressure,
                     float reportedPM25, float reportedPM10, float reportedTEMP, float reportedHUM, float reportedPRES) {

  Serial.print(timestamp); // RTC time
  Serial.print("\t");
  Serial.print(String(rawPM25) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  Serial.print(String(rawPM10) + "\t");//PM10 mass concentration from SDS011 in [μg/m3]
  Serial.print(String(temperature) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  Serial.print(String(humidity) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
  Serial.print(String(pressure) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]

  if (readIndex >= numReadings) {

#ifdef UseAverage
    Serial.print(String(reportedPM25) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    Serial.print(String(reportedPM10) + "\t"); //PM10 mass concentration from SDS011 in [μg/m3]
    Serial.print(String(reportedTEMP) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    Serial.print(String(reportedHUM) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    Serial.print(String(reportedPRES) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
    //        readIndex = 0;
#endif

    Serial.print("\r\n");
  }
}
