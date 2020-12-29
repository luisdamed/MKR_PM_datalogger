void GetRTCTime () {

#ifdef DebugMessages
  Serial.println("\n\nGetting current time from RTC");
#endif

  uint8_t hour = (rtc.getHours() + GMT);
  uint8_t mins = (rtc.getMinutes());
  uint8_t sec = (rtc.getSeconds());
  uint8_t day = (rtc.getDay());
  uint8_t month = (rtc.getMonth());
  uint16_t year = (rtc.getYear());
  uint8_t daysInFebruary = 28 + leapyear(year);
  if (hour >= 24) {
    calendartime (hour, day, month, year, daysInFebruary);
  }
  snprintf_P(timestamp, sizeof(timestamp), PSTR("%02u/%02u/%u %02u:%02u:%02u"), day, month, year, hour, mins, sec);
}

int leapyear(uint16_t year)
{
  if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)
    return 1;  // This is a leap year
  else
    return 0;
}


void calendartime (uint8_t hour, uint8_t  day, uint8_t month, uint16_t year, uint8_t daysInFebruary)
{
  //Workaround for RTC Zero library ver 1.6.0 bug
  hour = hour - 24;
  day = day + 1;

  //Move from one month to another or change year
  if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10) {
    //If the month has 31 days, change month when the day is already 31 (except for new year)
    if (day > 31) {
      day = 1;
      month = month + 1;
    }
  }
  else if  (month == 4 || month == 6 || month == 9 || month == 11) {
    //If the month has 30 days, change month when the day is already 30
    if (day > 30) {
      day = 1;
      month = month + 1;
    }
  }
  else if (month == 2) {
    //Move from February to March
    if (day > daysInFebruary) {
      day = 1;
      month = month + 1;
    }
  }
  else if (month == 12) {
    //Move from December to January of the next year
    if (day > 31) {
      day = 1;
      month = 1;
      year = year + 1;
    }
  }
}
