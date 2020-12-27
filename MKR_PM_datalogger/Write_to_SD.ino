//bool Write_to_SD(char* timestamp, float rawPM25, float rawPM10, float temperature, float humidity, float pressure,
//                 float reportedPM25, float reportedPM10, float reportedTEMP, float reportedHUM, float reportedPRES) {
//
//
//  myFile = SD.open("log.txt", FILE_WRITE); // create a test file
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    #ifdef DebugMessages
//    Serial.print("Writing to log.txt...\n");
//    #endif
//    myFile.print(timestamp); // RTC time
//    myFile.print("\t");
//
//    
//    myFile.print(String(rawPM25) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
//    myFile.print(String(rawPM10) + "\t");//PM10 mass concentration from SDS011 in [μg/m3]
//    myFile.print(String(temperature) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
//    myFile.print(String(humidity) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
//    myFile.print(String(pressure) + "\t");//PM2.5 mass concentration from SDS011 in [μg/m3]
//    //      Serial.print("\n" + String(readIndex));
//
//    if (readIndex >= numReadings) {
//
//#ifdef UseAverage
//      myFile.print(String(reportedPM25) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
//      myFile.print(String(reportedPM10) + "\t"); //PM10 mass concentration from SDS011 in [μg/m3]
//      myFile.print(String(reportedTEMP) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
//      myFile.print(String(reportedHUM) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
//      myFile.print(String(reportedPRES) + "\t"); //PM2.5 mass concentration from SDS011 in [μg/m3]
//#endif
//
//
//    }
//    myFile.print("\n");
//    // close the file:
//    myFile.close();
//  }
//}
