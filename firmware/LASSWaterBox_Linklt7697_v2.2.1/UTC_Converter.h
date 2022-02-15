#ifndef _UTC_converter_H_
#define _UTC_converter_H_

#include <Arduino.h>

class UTC_Converter
{
public:
  UTC_Converter(bool _isNTP = false);
  ~UTC_Converter();

  unsigned int year;
  unsigned int  month;
  unsigned int day;
  unsigned int hour;
  unsigned int minute;
  unsigned int second;
  unsigned int WeeksOfYear;          // 每年第幾周
  unsigned int DaysOfWeek;           // 每星期第幾天
  unsigned int DaysOfYear;           // 暫存用：一年剩於日數

  void setUTC(unsigned long _t);    // 1970.1.1 00:00:00開始計算
  void setNTP(unsigned long _t);    // 1900.1.1 00:00:00開始計算
  void setTimeZone(int _tz = 0);

  unsigned long getUTC();

  unsigned long UTCtime;            // UTC的時間(暫存用)
  char* Types;                      // 格式總類說明文字

private:
  bool isNTP;
  bool isLeapYear;

  const int LeepNumber1970 = 2;                         // 1972是潤年，所以1970
  const int LeepDays[4] = { 366,365,365,365 };
  int monthDays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

  const unsigned long _utc_Datum = 2208988800UL;        // UTC計算基準(NTP 跟 UTC不同)
  const int week_days[7] = { 4,5,6,0,1,2,3 };           // 1970 1/1 是禮拜4

  char* UTC = "UTC Time (since  1970.1.1 00:00:00)";
  char* NTP = "NTP Time (since  1900.1.1 00:00:00)";

  unsigned long _Ds;             // 暫存用：日數(sinece 1970)
  unsigned long _Ss;             // 暫存用：剩餘秒數

  long _TempD;                    // 暫存用：日數計算臨時區

  int tz;

  int getHour(unsigned long _s);
  int getMinute(unsigned long _s);
  int getSecond(unsigned long _s);

  unsigned int getDate(unsigned long _d);
  int getWeeks(unsigned long _d);
  int getDaysOfWeek(unsigned long _d);
};

#endif
