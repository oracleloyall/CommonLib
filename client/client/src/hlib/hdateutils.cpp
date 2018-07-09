/*
 * hdateutils.cpp
 *
 *  Created on: 2013-9-22
 *      Author: root
 */

#ifndef HDATEUTILS_CPP_
#define HDATEUTILS_CPP_

#include "hdateutils.hpp"
#include "hhmsi.hpp"

// global vars

typedef PHElement PHE;

Word MonthDays[2][12] =
  {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

Word CDayMap[7] = {7, 1, 2, 3, 4, 5, 6};

typedef THHMSI* PHHMSI;

PHHMSI MonthTable;
PHHMSI WeekdayTable;


const string WEEK_DAYS[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const string MONTHS[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// initializations

namespace hdateutils
{
	class TUnitController
	{
		public:
		TUnitController();
		~TUnitController();
	};

	TUnitController::TUnitController()
	{
		// Initialization
		MonthTable = new THHMSI(12);
		WeekdayTable = new THHMSI(7);
		for(int i = 0; i < 7; i++) WeekdayTable->AddS(WEEK_DAYS[i], i);
		for(int i = 0; i < 12; i++) MonthTable->AddS(MONTHS[i], i);
	}

	TUnitController::~TUnitController()
	{
		// Finalization();
		delete MonthTable;
		delete WeekdayTable;
	}

	TUnitController ThisUnit;
}

// functions

bool IsLeapYear(Word Year)
{
  return ((Year % 4) == 0) && (((Year % 100) != 0) || ((Year % 400) == 0));
}

void DivMod(int T, Word D, pWord I, pWord X)
{
  *I = T / (int)D;
  *X = T - D * (*I);
}

void DateTimeToTimeStamp(double d, PTimeStamp dt)
{
  dt->Date = floor(d);
  dt->Time = (d - dt->Date) * (double(1) / (double(1) / 24 / 60 / 60 / 1000)) + 0.1;
  dt->Date += DateDelta;
}

bool DecodeDateFully(const double DateTime, pWord Year, pWord Month, pWord Day, pWord DOW)
{
  Word Y, M, D, I;
  bool ret;
  PDayTable DayTable;
  TTimeStamp ts;
  DateTimeToTimeStamp(DateTime, &ts);

  int T = ts.Date;
  if (T <= 0)
  {
    *Year = 0;
    *Month = 0;
    *Day = 0;
    *DOW = 0;
    return false;
  }
  else
  {
    *DOW = Word(T % 7 + 1);
    T--;
    Y = 1;
    while (T >= D400)
    {
      T -= D400;
      Y += 400;
    }
    DivMod(T, D100, &I, &D);
    if (I == 4)
    {
      I--;
      D += D100;
    }
    Y += I * 100;
    DivMod(D, D4, &I, &D);
    Y += I * 4;
    DivMod(D, D1, &I, &D);
    if (I == 4)
    {
      I--;
      D += D1;
    }
    Y += I;
    ret = IsLeapYear(Y);
    if (ret)
      DayTable = &MonthDays[1];
    else
      DayTable = &MonthDays[0];
    M = 1;
    while (true)
    {
      I = (*DayTable)[M - 1];
      if (D < I) break;
      D -= I;
      M++;
    }
    *Year = Y;
    *Month = M;
    *Day = D + 1;
  }
  return ret;
}

void DecodeDate(const double DateTime, pWord Year, pWord Month, pWord Day)
{
  Word Dummy;
  DecodeDateFully(DateTime, Year, Month, Day, &Dummy);
}

Word YearOf(const double AValue)
{
  Word LYear, LMonth, LDay;
  DecodeDate(AValue, &LYear, &LMonth, &LDay);
  return LYear;
}

Word MonthOf(const double AValue)
{
  Word LYear, LMonth, LDay;
  DecodeDate(AValue, &LYear, &LMonth, &LDay);
  return LMonth;
}

Word DayOf(const double AValue)
{
  Word LYear, LMonth, LDay;
  DecodeDate(AValue, &LYear, &LMonth, &LDay);
  return LDay;
}

void DecodeTime(const double DateTime, pWord Hour, pWord Min, pWord Sec, pWord MSec)
{
  TTimeStamp ts;
  Word MinCount, MSecCount;
  DateTimeToTimeStamp(DateTime, &ts);
  DivMod(ts.Time, SecsPerMin * MSecsPerSec, &MinCount, &MSecCount);
  DivMod(MinCount, MinsPerHour, Hour, Min);
  DivMod(MSecCount, MSecsPerSec, Sec, MSec);
}

Word HourOf(const double AValue)
{
  Word LHour, LMinute, LSecond, LMilliSecond;
  DecodeTime(AValue, &LHour, &LMinute, &LSecond, &LMilliSecond);
  return LHour;
}

Word MinuteOf(const double AValue)
{
  Word LHour, LMinute, LSecond, LMilliSecond;
  DecodeTime(AValue, &LHour, &LMinute, &LSecond, &LMilliSecond);
  return LMinute;
}

Word SecondOf(const double AValue)
{
  Word LHour, LMinute, LSecond, LMilliSecond;
  DecodeTime(AValue, &LHour, &LMinute, &LSecond, &LMilliSecond);
  return LSecond;
}

Word MilliSecondOf(const double AValue)
{
  Word LHour, LMinute, LSecond, LMilliSecond;
  DecodeTime(AValue, &LHour, &LMinute, &LSecond, &LMilliSecond);
  return LMilliSecond;
}

string FormatDateTime(const string FormatString, double ADate)
{
    // 1. prepare data
    Word Year, Month, Day, Hour, Minute, Sec, MSec;
    DecodeDate(ADate, &Year, &Month, &Day);
    DecodeTime(ADate, &Hour, &Minute, &Sec, &MSec);
    string yyyy = IntToStr(Year);
    string yy = IntToStr(Year % 100);
    string m = IntToStr(Month);
    string mm = (m.length() == 1) ? (string("0") + m) : m;
    string d = IntToStr(Day);
    string dd = (d.length() == 1) ? (string("0") + d) : d;
    string h = IntToStr(Hour);
    string hh = (h.length() == 1) ? (string("0") + h) : h;
    string n = IntToStr(Minute);
    string nn = (n.length() == 1) ? (string("0") + n) : n;
    string s = IntToStr(Sec);
    string ss = (s.length() == 1) ? (string("0") + s) : s;
    string z = IntToStr(MSec);
    char buf[3] = {'0','0','0'};
    memcpy(&buf[3 - z.length()], z.c_str(), z.length());
    string zzz = string(buf);
    // 2. output
    string x;
    x.assign(FormatString);
    StringReplace(x, string("yyyy"), yyyy);
    StringReplace(x, string("yy"), yy);
    StringReplace(x, string("mm"), mm);
    StringReplace(x, string("m"), m);
    StringReplace(x, string("dd"), dd);
    StringReplace(x, string("d"), d);
    StringReplace(x, string("hh"), hh);
    StringReplace(x, string("h"), h);
    StringReplace(x, string("nn"), nn);
    StringReplace(x, string("n"), n);
    StringReplace(x, string("ss"), ss);
    StringReplace(x, string("s"), s);
    StringReplace(x, string("zzz"), zzz);
    StringReplace(x, string("z"), z);
    // 3. free
    //free(z); free(s); free(n); free(h);
    //free(d); free(m); free(yy); free(yyyy);
    return x;
}

bool TryEncodeDate(Word Year, Word Month, Word Day, pDouble Date)
{
  PDayTable DayTable = IsLeapYear(Year) ? &MonthDays[1] : &MonthDays[0];
  if ((Year >= 1) && (Year <= 9999) && (Month >= 1) && (Month <= 12) &&
    (Day >= 1) && (Day <= (*DayTable)[Month - 1]))
  {
    for (int i = 1 ; i <  Month; i++) Day += (*DayTable)[i - 1];
    int i = Year - 1;
    *Date = i * 365 + (i >> 2) - i / 100 + i / 400 + Day - DateDelta;
    return true;
  }
  return false;
}

double EncodeDate(Word Year, Word Month, Word Day)
{
	double ret;
    if (!TryEncodeDate(Year, Month, Day, &ret)) return 0;
    return ret;
}

double Date()
{
	time_t T;
	tm UT;
	time(&T);
	localtime_r(&T, &UT);
	return EncodeDate(UT.tm_year + 1900, UT.tm_mon + 1, UT.tm_mday);
}

bool TryEncodeTime(Word Hour, Word Min, Word Sec, Word MSec, pDouble Time)
{
  if ( (Hour < HoursPerDay) and (Min < MinsPerHour) and (Sec < SecsPerMin) and (MSec < MSecsPerSec) )
  {
    *Time = ((double)Hour * ((double)MinsPerHour * SecsPerMin * MSecsPerSec) +
    		(double)Min * (SecsPerMin * MSecsPerSec) +
    		(double)Sec * MSecsPerSec +
    		(double)MSec) / MSecsPerDay;
    return true;
  }
  return false;
}

double EncodeTime(Word Hour, Word Min, Word Sec, Word MSec)
{
  double ret;
  if (!TryEncodeTime(Hour, Min, Sec, MSec, &ret)) return false;
  return ret;
}

double Time()
{
	time_t T;
	timeval TV;
	tm UT;
	gettimeofday(&TV, NULL);
	T = TV.tv_sec;
	localtime_r(&T, &UT);
	return EncodeTime(UT.tm_hour, UT.tm_min, UT.tm_sec, TV.tv_usec / 1000);
}

double Now()
{
	time_t T;
	timeval TV;
	tm UT;
	gettimeofday(&TV, NULL);
	T = TV.tv_sec;
	localtime_r(&T, &UT);
	return EncodeDate(UT.tm_year + 1900, UT.tm_mon + 1, UT.tm_mday) +
	    EncodeTime(UT.tm_hour, UT.tm_min, UT.tm_sec, TV.tv_usec / 1000);
}

bool TryStrToWeekDay(const string& s, pWord wd)
{
	PHE p = WeekdayTable->FindS(s);
	if(!p) return false;
	*wd = p->Value + 1;
	return true;
}

bool TryStrToMonth(const string& s, pWord wd)
{
	PHE p = MonthTable->FindS(s);
	if(!p) return false;
	*wd = p->Value + 1;
	return true;
}

void DecodeDateTime(const double AValue, pWord AYear, pWord AMonth, pWord ADay,
		            pWord AHour, pWord AMinute, pWord ASecond, pWord AMilliSecond)
{
	DecodeDate(AValue, AYear, AMonth, ADay);
	DecodeTime(AValue, AHour, AMinute, ASecond, AMilliSecond);
}

void DecodeDateWeek(const double AValue, pWord AYear, pWord AWeekOfYear, pWord ADayOfWeek)
{
	Word LMonth, LDay;
    bool LLeap = DecodeDateFully(AValue, AYear, &LMonth, &LDay, ADayOfWeek);
    *ADayOfWeek = CDayMap[*ADayOfWeek - 1];
    double LStart = EncodeDate(*AYear, 1, 1);
    int LDayOfYear = AValue - LStart + 1;
    Word LStartDayOfWeek = DayOfTheWeek(LStart);
    if (LStartDayOfWeek > 4)
    	LDayOfYear -= (8 - LStartDayOfWeek);
    else
        LDayOfYear += (LStartDayOfWeek - 1);
    if (LDayOfYear <= 0)
    	DecodeDateWeek(LStart - 1, AYear, AWeekOfYear, &LDay);
    else
    {
    	*AWeekOfYear = LDayOfYear / 7;
    	if (LDayOfYear % 7) (*AWeekOfYear)++;
    	if (*AWeekOfYear > 52)
    	{
    		Word LEndDayOfWeek = LStartDayOfWeek;
    		if (LLeap)
    		{
    			if (LEndDayOfWeek == 7)
    				LEndDayOfWeek = 1;
    			else
    				LEndDayOfWeek++;
    		}
    		if (LEndDayOfWeek < 4)
    		{
    			(*AYear)++;
    			*AWeekOfYear = 1;
    		}
    	}
    }
}

Word DayOfTheWeek(const double AValue)
{
	TTimeStamp d;
	DateTimeToTimeStamp(AValue, &d);
    return (d.Date - 1) % 7 + 1;
}

string DateToGMTStr(double dt, bool GMTFix)
{
    Word y, m, d, h, n, s, z, w, x;
    if (GMTFix) dt -= LOCAL_TIME_DELTA;
    DecodeDateTime(dt, &y, &m, &d, &h, &n, &s, &z);
    DecodeDateWeek(dt, &y, &w, &x);
    char* buf = new char[256];
    memset(buf, 0, 256);
    string t = FormatDateTime("hh:nn:ss", dt);
    sprintf(buf, "%s, %d %s %d %s GMT", WEEK_DAYS[x - 1].c_str(), d, MONTHS[m - 1].c_str(), y, t.c_str());
    string a = string(buf);
    free(buf);
    return a;
}

bool TryGMTStrToDate(const string s, pDouble dt, bool GMTFix)
{
  const string SUFFIX_WEEKDAY = ", ";
  const string PREFIX_GMT = " GMT";
  int l, i, j, k;
  Word y, m, d, h, n, z;
  //Fri, 14 Jan 2011 05:02:24 GMT
  l = s.length();
  if ((l > 28) && (l < 31))
  {
    if (s.compare(3, 2, SUFFIX_WEEKDAY.c_str())) return false;
    if (s.compare(l - 4, 4, PREFIX_GMT.c_str())) return false;
    j = 0;
    k = 6;
    for (i = 6; i <= (l - 3); i++)
    {
      char c = s.at(i - 1);
      if ((c == ' ') || (c == ':'))
      {
        switch (++j)
        {
          case 1:
          {
        	 if (!TryStrToWord(s.substr(k - 1, i - k), &d)) return false;
        	 break;
          }
          case 2:
          {
        	 if (!TryStrToMonth(s.substr(k - 1, i - k), &m)) return false;
        	 break;
          }
          case 3:
          {
        	 if (!TryStrToWord(s.substr(k - 1, i - k), &y)) return false;
        	 break;
          }
          case 4:
          {
        	 if (!TryStrToWord(s.substr(k - 1, i - k), &h)) return false;
        	 break;
          }
          case 5:
          {
        	 if (!TryStrToWord(s.substr(k - 1, i - k), &n)) return false;
        	 break;
          }
          case 6:
          {
        	 if (!TryStrToWord(s.substr(k - 1, i - k), &z)) return false;
        	 break;
          }
       }
       k = i + 1;
     }
    }
    *dt = EncodeDateTime(y, m, d, h, n, z, 0);
    if (GMTFix) *dt += LOCAL_TIME_DELTA;
    return true;
  }
  return false;
}

bool TryEncodeDateTime(const Word AYear, const Word AMonth, const Word ADay, const Word AHour, const Word AMinute, const Word ASecond, const Word AMilliSecond, pDouble AValue)
{
  double LTime;
  if (TryEncodeDate(AYear, AMonth, ADay, AValue))
	if (TryEncodeTime(AHour, AMinute, ASecond, AMilliSecond, &LTime))
	{
      *AValue += LTime;
      return true;
	}
  return false;
}

double GMTStrToDate(const string s, bool GMTFix)
{
  double ret;
  if (!TryGMTStrToDate(s, &ret, GMTFix)) return 0; else return ret;
}

double EncodeDateTime(const Word AYear, const Word AMonth, const Word ADay, const Word AHour, const Word AMinute, const Word ASecond, const Word AMilliSecond)
{
  double ret = 0;
  if (!TryEncodeDateTime(AYear, AMonth, ADay, AHour, AMinute, ASecond, AMilliSecond, &ret))
    return 0;
  else
	return ret;
}

double StrToDateTime(const string z)
{
  Word y, m, d, h, n, s;
  double x = 0;
  if (z.length() == 14)
  {
    y = StrToInt(LeftStr(z, 4));
    m = StrToInt(MidStr(z, 5, 2));
    d = StrToInt(MidStr(z, 7, 2));
    h = StrToInt(MidStr(z, 9, 2));
    n = StrToInt(MidStr(z, 11, 2));
    s = StrToInt(RightStr(z, 2));
    x = EncodeDate(y, m, d) + EncodeTime(h, n, s, 0);
  }
  return x;
}

string FormatTinyTime(const double x)
{
  double t = x;
  string s = string("");
  if (t < 0)
  {
    t = 0 - t;
    s = "-";
  }
  if (t > 1)
  {
	char* buf = new char[32];
	memset(buf, 0, 32);
	sprintf(buf, "%.2f 秒", t);
    s += string(buf);
    delete [] buf;
    return s;
  }
  t *= 1000;
  if (t > 1)
  {
	char* buf = new char[32];
	memset(buf, 0, 32);
	sprintf(buf, "%.2f 毫秒", t);
	s += string(buf);
	delete [] buf;
	return s;
  }
  t *= 1000;
  if (t > 1)
  {
	char* buf = new char[32];
	memset(buf, 0, 32);
	sprintf(buf, "%.2f 微秒", t);
	s += string(buf);
	delete [] buf;
	return s;
  }
  t *= 1000;
  if (t > 1)
  {
	char* buf = new char[32];
	memset(buf, 0, 32);
	sprintf(buf, "%.2f 纳秒", t);
	s += string(buf);
	delete [] buf;
	return s;
  }
  t *= 1000;
  if (t > 1)
  {
	char* buf = new char[32];
	memset(buf, 0, 32);
	sprintf(buf, "%.2f 皮秒", t);
	s += string(buf);
	delete [] buf;
	return s;
  }
  char* buf = new char[32];
  memset(buf, 0, 32);
  sprintf(buf, "%.2f 飞秒", t);
  s += string(buf);
  delete [] buf;
  return s;
}

string FormatHugeTime(const double x, bool AutoAdjust)
{
  int i, j;
  double t = x;
  string s = "";
  string u;
  j = 2;
  if (t < 0)
  {
    t = 0 - t;
    u = "前";
  }
  else
    u = "";
  do {
    if (t >= 365)
    {
      i = (int)t / 365;
      s = s + IntToStr(i) + "年";
      t = t - i * 365;
      j--;
    }
    if (t >= 30)
    {
      i = (int)t / 30;
      s = s + IntToStr(i) + "个月";
      t = t - i * 30;
      j--;
      if (AutoAdjust && (j == 0)) break;
    }
    if (t >= 1)
    {
      i = (int)t;
      s = s + IntToStr(i) + "天";
      t = t - i;
      j--;
      if (AutoAdjust && (j == 0)) break;
    }
    if (t >= ((double)1 / 24))
    {
      i = (int)(t * 24);
      s = s + IntToStr(i) + "小时";
      t = t - ((double)1 / 24) * i;
      j--;
      if (AutoAdjust && (j == 0)) break;
    }
    if (t >= ((double)1 / 24 / 60))
    {
      i = (int)(t * 24 * 60);
      s = s + IntToStr(i) + "分";
      t = t - ((double)1 / 24 / 60) * i;
      j--;
      if (AutoAdjust && (j == 0)) break;
    }
    if (t >= H_SEC)
    {
      i = (int)(t * 24 * 60 * 60);
      s = s + IntToStr(i) + "秒";
      t = t - (double)i * H_SEC;
      j--;
      if (AutoAdjust && (j == 0)) break;
    }
    if (s.length() == 0) s = FormatTinyTime(t * H_SEC_R);
  } while (false);
  s = s + u;
  return s;
}

Int64 GetMicroSeconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (Int64)tv.tv_sec * 1000000 + tv.tv_usec;
}

Int64 GetMicroSecondsDraft()
{
    Int64 i;
    //gettimeofday((timeval*)(&i), NULL);
    //return i;
	timeval t;
	gettimeofday(&t, NULL);
	memcpy(&i, &t, 8);
	return i;
}

Cardinal GetSeconds()
{
	time_t tt;
	time(&tt);
	return tt;
}

Int64 GetTimestamp(bool GMTFix)
{
	if (GMTFix)
		return DateTimeToUnix(Now() + 1.0f / 3);
	else
		return DateTimeToUnix(Now());
}

Cardinal GetTickCount() // ms
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec % 604800) * 1000 + ((double)1 / 1000) * tv.tv_usec;
}

string FormatMS(Int64 dr, Int64 dn)
{
	struct timeval d1, d2;
    double x1, x2;
    if (!dn) dn = GetMicroSeconds();
    memcpy(&d2, &dr, 8);
    memcpy(&d1, &dn, 8);
    x1 = ((double)1 / 1000000) * d1.tv_usec + d1.tv_sec;
    x2 = ((double)1 / 1000000) * d2.tv_usec + d2.tv_sec;
    return FormatTinyTime(x2 - x1);
}

Int64 DateTimeToUnix(const double AValue)
{
	return round((AValue - UnixDateDelta) * SecsPerDay);
}

string DateTimeToGMT(const double AValue)
{
    return DateToGMTStr(AValue);
}

#endif /* HDATEUTILS_CPP_ */
