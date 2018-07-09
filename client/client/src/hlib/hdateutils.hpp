/*
 * hdateutils.hpp
 *
 *  Created on: 2013-9-22
 *      Author: root
 */

#ifndef HDATEUTILS_HPP_
#define HDATEUTILS_HPP_

#include "global.hpp"
#include "hpasutils.hpp"
#include "hhmsi.hpp"

const int D1 = 365;
const int D4 = D1 * 4 + 1;
const int D100 = D4 * 25 - 1;
const int D400 = D100 * 4 + 1;

const double LOCAL_TIME_DELTA = (double)1 / 3;
const double H_SEC = (double)1 / 24 / 60 / 60;
const double H_SEC_R = (double)1 / H_SEC;

typedef Word TDayTable[12];
typedef TDayTable* PDayTable;

typedef struct TTimeStamp
{
  int Time;
  int Date;
} *PTimeStamp;

typedef struct TCalcTimeStatus
{
  char Host[256];
  char IP[18];
  int Timeout;
  int ValidSample;
  double Variance;
  double AverageDeviation;
  double TimeDelta;
  char Error[256];
  bool AutoFix;
  bool SamplesQuantityControl;
  bool SamplesQualityControl;
} *PCalcTimeStatus;

extern Cardinal GetSeconds();
extern Int64 GetMicroSeconds();
extern string FormatDateTime(const string FormatString, double ADate);
extern void DateTimeToTimeStamp(double d, PTimeStamp dt);
extern double Now();
extern double Date();
extern double Time();
extern double EncodeTime(Word Hour, Word Min, Word Sec, Word MSec);
extern bool TryEncodeTime(Word Hour, Word Min, Word Sec, Word MSec, pDouble Time);
extern double EncodeDate(Word Year, Word Month, Word Day);
extern bool TryEncodeDate(Word Year, Word Month, Word Day, pDouble Date);
extern Word MilliSecondOf(const double AValue);
extern Word SecondOf(const double AValue);
extern Word MinuteOf(const double AValue);
extern Word HourOf(const double AValue);
extern void DecodeTime(const double DateTime, pWord Hour, pWord Min, pWord Sec, pWord MSec);
extern Word DayOf(const double AValue);
extern Word MonthOf(const double AValue);
extern Word YearOf(const double AValue);
extern void DecodeDate(const double DateTime, pWord Year, pWord Month, pWord Day);
extern bool TryStrToWeekDay(const string& s, pWord wd);
extern bool TryStrToMonth(const string& s, pWord wd);
extern string DateToGMTStr(double dt, bool GMTFix = true);
extern void DecodeDateTime(const double AValue, pWord AYear, pWord AMonth, pWord ADay, pWord AHour, pWord AMinute, pWord ASecond, pWord AMilliSecond);
extern void DecodeDateWeek(const double AValue, pWord AYear, pWord AWeekOfYear, pWord ADayOfWeek);
extern Word DayOfTheWeek(const double AValue);
extern bool TryEncodeDateTime(const Word AYear, const Word AMonth, const Word ADay, const Word AHour, const Word AMinute, const Word ASecond, const Word AMilliSecond, pDouble AValue);
extern double EncodeDateTime(const Word AYear, const Word AMonth, const Word ADay, const Word AHour, const Word AMinute, const Word ASecond, const Word AMilliSecond);
extern bool TryGMTStrToDate(const string s, pDouble dt, bool GMTFix = true);
extern double GMTStrToDate(const string s, bool GMTFix);
extern double StrToDateTime(const string z);
extern string FormatTinyTime(const double x);
extern string FormatHugeTime(const double x, bool AutoAdjust = true);
extern string FormatMS(Int64 dr, Int64 dn);
extern Int64 DateTimeToUnix(const double AValue);
extern Cardinal GetTickCount();
//extern double UnixToDateTime(const Int64 AValue);
extern string DateTimeToGMT(const double AValue);
extern Int64 GetTimestamp(bool GMTFix = false);
extern Int64 GetMicroSecondsDraft();
#endif /* HDATEUTILS_HPP_ */
