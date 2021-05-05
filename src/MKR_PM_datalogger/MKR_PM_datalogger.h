
// GNU GENERAL PUBLIC LICENSE
//                      Version 3, 29 June 2007
//
//  Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
//  Everyone is permitted to copy and distribute verbatim copies
//  of this license document, but changing it is not allowed.

#ifndef __MKR_PM_DATALOGGER__
#define __MKR_PM_DATALOGGER__

#include <Arduino.h>
#include <Arduino_MKRENV.h>
#include <SPI.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include <ArduinoUniqueID.h>
#include "SdsDustSensor.h"


class MKR_PM_datalogger {

    public:

        // Define global variables

        /*----------------------Work parameters--------------------*/
        int g_numReadings;
        int g_sensorWorkPeriod;
        int g_lowPowerTime;

        /*----------------------Raw values-------------------------*/
        float g_rawPM10 = 0;          // PM10  mass concentration in ug/m3
        float g_rawPM25 = 0;          // PM2.5 mass concentration in ug/m3
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
        PmResult pm; //Variable to store PM resutls from query
        
        #ifdef WriteToSD
        File myFile;                // text file
        #endif

        #ifdef UseNTPTime
        RTCZero rtc;                // real time clock internal to the MKR WiFi 1010
        int status = WL_IDLE_STATUS;
        #endif
        

    
        //Constructor for MKR_PM_datalogger object
        MKR_PM_datalogger (int numReadings = 10, int sensorWorkPeriod = 1, int lowPowerTime = 300);
        


        //Methods
        void begin(int baudRate = 9600);
        void GetRTCTime();
        void wakeUp_MKR_PM();
        void query_MKR_PM();
        void read_ENV_MKR();
        void compute_average();
        void printResults();
        void setSleepMode(int lowPowerTime);
        



    private:




};






#endif
