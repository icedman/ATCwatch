/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Arduino.h"
#include "sleep.h"
#include "time.h"
#include "heartrate.h"
#include "inputoutput.h"
#include <TimeLib.h>

time_data_struct time_data;

void init_time() {
    int year = 2020;
    int month = 1;
    int day = 1;
    int hr = 8;
    int min = 0;
    int sec = 0;
    setTime( hr, min, sec, day, month, year);
}

time_data_struct get_time() {
    time_data.year = year();
    time_data.month = month();
    time_data.day = day();
    time_data.hr = hour();
    time_data.min = minute();
    time_data.sec = second();
    return time_data;
}

void SetDateTimeString(String datetime) {
    int year = datetime.substring(0, 4).toInt();
    int month = datetime.substring(4, 6).toInt();
    int day = datetime.substring(6, 8).toInt();
    int hr = datetime.substring(8, 10).toInt();
    int min = datetime.substring(10, 12).toInt();
    int sec = datetime.substring(12, 14).toInt();
    setTime( hr, min, sec, day, month, year);
}

void SetDate(int year, int month, int day) {
    time_data = get_time();
    setTime( time_data.hr, time_data.min, time_data.sec, day, month, year);
}

void SetTime(int hr, int min) {
    time_data = get_time();
    setTime( hr, min, 0, time_data.day, time_data.month, time_data.year);
}

String GetDateTimeString() {
    String datetime = String(year());
    if (month() < 10) datetime += "0";
    datetime += String(month());
    if (day() < 10) datetime += "0";
    datetime += String(day());
    if (hour() < 10) datetime += "0";
    datetime += String(hour());
    if (minute() < 10) datetime += "0";
    datetime += String(minute());
    return datetime;
}


/* 
  Put current time with seconds into string provided
 */
void getTimeWithSecs(char* str) {
  sprintf(str, "%02d:%02d:%02d", hour(), minute(), second());
}

/* 
  Put the current time without seconds into string provided
 */
void getTime(char* str) {
  sprintf(str, "%02d:%02d", hour(), minute());
}

/* 
  Put the current date into string provided
 */
void getDate(char* str) {
  sprintf(str, "%02d.%02d.%04d", day(), month(), year());
}

/* 
  Put current day into string provided
  Memset is used to make buffer all spaces initially, since the string length is variable
 */
void getDay(char* str) {
  memset(str, ' ', 9);
  switch (getDayOfWeek()) {
    case 0:
      sprintf(str, "Sunday");
      break;
    case 1:
      sprintf(str, "Monday");
      break;
    case 2:
      sprintf(str, "Tuesday");
      break;
    case 3:
      sprintf(str, "Wednesday");
      break;
    case 4:
      sprintf(str, "Thursday");
      break;
    case 5:
      sprintf(str, "Friday");
      break;
    case 6:
      sprintf(str, "Saturday");
      break;
    default:
      sprintf(str, "UNK");
      break;
  }
}

/* 
  Get the weekday from a given day month and year
 */
uint8_t getDayOfWeek(int d, int m, int y) {
  int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

/* 
  Get the current weekday
 */
uint8_t getDayOfWeek() {
  return getDayOfWeek(day(), month(), year());
}

/*
  Set the time (just a wrapper for the library implementation (here for convenience))
*/
void setTimeWrapper(int yr, int mth, int _day, int hr, int _min, int sec) {
  setTime(hr, _min, sec, _day, mth, yr);
}