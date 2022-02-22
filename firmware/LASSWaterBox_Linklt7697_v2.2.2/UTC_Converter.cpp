#include "UTC_converter.h"

UTC_Converter::UTC_Converter(bool _isNTP)
{
  isNTP = _isNTP;

  if (isNTP) {
    Types = NTP;
  }
  else {
    Types = UTC;
  }
}


void UTC_Converter::setUTC(unsigned long _t)
{
  if (isNTP) _t -= _utc_Datum;            // 換算成UTC
  UTCtime = _t;
  _Ds = _t / 86400L;                      // 總日數
  _Ss = _t % 86400L;                      // 剩餘秒數

  hour = getHour(_Ss);
  minute = getMinute(_Ss);
  second = getSecond(_Ss);

  DaysOfYear = getDate(_Ds);             // 把 年,月,日計算出來，再返回今年第幾天
  WeeksOfYear = getWeeks(DaysOfYear);
  DaysOfWeek = getDaysOfWeek(_Ds);
}

int UTC_Converter::getHour(unsigned long _s)
{
  return(_s / 3600) % 24;
}

int UTC_Converter::getMinute(unsigned long _s)
{
  return (_s / 60) % 60;
}

int UTC_Converter::getSecond(unsigned long _s)
{
  return _s % 60;
}


unsigned int  UTC_Converter::getDate(unsigned long _d)
{
  int _Infer = _d / 365;  // 先抓一個預估年數
  int _c = 0;

  _TempD = _d;
  unsigned int _result;

  // 用輪詢的方式把年份算出來
  for (int _i = 0; _i < _Infer; _i++)
  {
    _c = (LeepNumber1970 + _i) % 4;

    if (_c == 3) isLeapYear = true;
    else        isLeapYear = false;

    _TempD -= LeepDays[_c];

    if (_TempD > 0) {
      _result = _TempD;
      year = 1970 + _i+1;
    }
    else {
      break;
    }
  }

  // 是閏年的話，修改二月份的日數
  if (isLeapYear) monthDays[1] = 29;

  // 計算月份和幾號
  for (int _i = 0; _i < 12; _i++) 
  {
    day = _TempD+1;
    month = _i + 1;
    _TempD -= monthDays[_i];
    if (_TempD < 0) break;
  }

  return _result;
}


int UTC_Converter::getWeeks(unsigned long _d)  // 計算第幾周
{
  return _d / 7;
}

int UTC_Converter::getDaysOfWeek(unsigned long _d)    // 計算星期幾
{
  /* 1970 1/1 是禮拜4 */
  return week_days[_d % 7];
}

void UTC_Converter::setTimeZone(int _tz)
{
  unsigned long zone =  UTCtime + _tz * 3600;

  _Ds = zone / 86400L;      // 總日數
  _Ss = zone % 86400L;    // 剩餘秒數

  hour = getHour(_Ss);
  minute = getMinute(_Ss);
  second = getSecond(_Ss);

  DaysOfYear = getDate(_Ds);                  // 把 年,月,日計算出來，再返回今年第幾天
  WeeksOfYear = getWeeks(DaysOfYear);
  DaysOfWeek = getDaysOfWeek(_Ds);

}

UTC_Converter::~UTC_Converter() {
};
