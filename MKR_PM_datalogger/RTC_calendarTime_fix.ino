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
  uint8_t daysInFebruary = 28 + leapyear(&year); 
  if (hour >= 24) {
    calendartime (&hour, &day, &month, &year, &daysInFebruary);
  }
  snprintf_P(timestamp, sizeof(timestamp), PSTR("%02u/%02u/%u %02u:%02u:%02u"), day, month, year, hour, mins, sec);
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
