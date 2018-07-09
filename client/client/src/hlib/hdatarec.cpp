/*
 * hdatarec.cpp
 *
 *  Created on: 2013-9-21
 *      Author: root
 */

#ifndef HDATAREC_CPP_
#define HDATAREC_CPP_

#include "hdatarec.hpp"

void AddBA(PHData src, PHData dst)
{
  double i = dst->Data.AByte;
  double j = src->Data.AByte;
  i = i * dst->Count + j * src->Count;
  dst->Count += src->Count;
  dst->Data.AByte = i / dst->Count + 0.5;
}

void AddBR(PHData src, PHData dst)
{
  dst->Data.AByte = src->Data.AByte;
  dst->Count += src->Count;
}

void AddBM(PHData src, PHData dst)
{
  dst->Data.AByte = max(src->Data.AByte, dst->Data.AByte);
  dst->Count += src->Count;
}

void AddBS(PHData src, PHData dst)
{
  dst->Data.AByte = min(src->Data.AByte, dst->Data.AByte);
  dst->Count += src->Count;
}

void AddBT(PHData src, PHData dst)
{
  dst->Data.AByte += src->Data.AByte;
  dst->Count += src->Count;
}

void AddWA(PHData src, PHData dst)
{
  double i = dst->Data.AWord;
  double j = src->Data.AWord;
  i = i * dst->Count + j * src->Count;
  dst->Count += src->Count;
  dst->Data.AWord = i / dst->Count + 0.5;
}

void AddWR(PHData src, PHData dst)
{
  dst->Data.AWord = src->Data.AWord;
  dst->Count += src->Count;
}

void AddWM(PHData src, PHData dst)
{
  dst->Data.AWord = max(src->Data.AWord, dst->Data.AWord);
  dst->Count += src->Count;
}

void AddWS(PHData src, PHData dst)
{
  dst->Data.AWord = min(src->Data.AWord, dst->Data.AWord);
  dst->Count += src->Count;
}

void AddWT(PHData src, PHData dst)
{
  dst->Data.AWord += src->Data.AWord;
  dst->Count += src->Count;
}

void AddCA(PHData src, PHData dst)
{
  double i = dst->Data.ADWord;
  double j = src->Data.ADWord;
  i = i * dst->Count + j * src->Count;
  dst->Count += src->Count;
  dst->Data.ADWord = i / dst->Count + 0.5;
}

void AddCR(PHData src, PHData dst)
{
  dst->Data.ADWord = src->Data.ADWord;
  dst->Count += src->Count;
}

void AddCM(PHData src, PHData dst)
{
  dst->Data.ADWord = max(src->Data.ADWord, dst->Data.ADWord);
  dst->Count += src->Count;
}

void AddCS(PHData src, PHData dst)
{
  dst->Data.ADWord = min(src->Data.ADWord, dst->Data.ADWord);
  dst->Count += src->Count;
}

void AddCT(PHData src, PHData dst)
{
  dst->Data.ADWord += src->Data.ADWord;
  dst->Count += src->Count;
}

void AddDA(PHData src, PHData dst)
{
  double i = dst->Data.ADouble * dst->Count + src->Data.ADouble * src->Count;
  dst->Count += src->Count;
  dst->Data.ADouble = i / dst->Count;
}

void AddDR(PHData src, PHData dst)
{
  dst->Data.ADouble = src->Data.ADouble;
  dst->Count += src->Count;
}

void AddDM(PHData src, PHData dst)
{
  dst->Data.ADouble = max(src->Data.ADouble, dst->Data.ADouble);
  dst->Count += src->Count;
}

void AddDS(PHData src, PHData dst)
{
  dst->Data.ADouble = min(src->Data.ADouble, dst->Data.ADouble);
  dst->Count += src->Count;
}

void AddDT(PHData src, PHData dst)
{
  dst->Data.ADouble = dst->Data.ADouble + src->Data.ADouble;
  dst->Count += src->Count;
}

void AddIA(PHData src, PHData dst)
{
  double i = dst->Data.AInt64;
  double j = src->Data.AInt64;
  i = i * dst->Count + j * src->Count;
  dst->Count += src->Count;
  dst->Data.AInt64 = i / dst->Count + 0.5;
}

void AddIR(PHData src, PHData dst)
{
  dst->Data.AInt64 = src->Data.AInt64;
  dst->Count += src->Count;
}

void AddIM(PHData src, PHData dst)
{
  dst->Data.AInt64 = max(src->Data.AInt64, dst->Data.AInt64);
  dst->Count += src->Count;
}

void AddIS(PHData src, PHData dst)
{
  dst->Data.AInt64 = min(src->Data.AInt64, dst->Data.AInt64);
  dst->Count += src->Count;
}

void AddIT(PHData src, PHData dst)
{
  dst->Data.AInt64 += src->Data.AInt64;
  dst->Count += src->Count;
}

void AddSA(PHData src, PHData dst)
{
  float i = dst->Data.AFloat * dst->Count + src->Data.AFloat * src->Count;
  dst->Count += src->Count;
  dst->Data.AFloat = i / dst->Count;
}

void AddSR(PHData src, PHData dst)
{
  dst->Data.AFloat = src->Data.AFloat;
  dst->Count += src->Count;
}

void AddSM(PHData src, PHData dst)
{
  dst->Data.AFloat = max(src->Data.AFloat, dst->Data.AFloat);
  dst->Count += src->Count;
}

void AddSS(PHData src, PHData dst)
{
  dst->Data.AFloat = min(src->Data.AFloat, dst->Data.AFloat);
  dst->Count += src->Count;
}

void AddST(PHData src, PHData dst)
{
  dst->Data.AFloat = dst->Data.AFloat + src->Data.AFloat;
  dst->Count += src->Count;
}

// add by hibiki at 22th sep, 2013

void AddiA(PHData src, PHData dst)
{
  double i = dst->Data.AInt;
  double j = src->Data.AInt;
  i = i * dst->Count + j * src->Count;
  dst->Count += src->Count;
  dst->Data.AInt = i / dst->Count + 0.5;
}

void AddiR(PHData src, PHData dst)
{
  dst->Data.AInt = src->Data.AInt;
  dst->Count += src->Count;
}

void AddiM(PHData src, PHData dst)
{
  dst->Data.AInt = max(src->Data.AInt, dst->Data.AInt);
  dst->Count += src->Count;
}

void AddiS(PHData src, PHData dst)
{
  dst->Data.AInt = min(src->Data.AInt, dst->Data.AInt);
  dst->Count += src->Count;
}

void AddiT(PHData src, PHData dst)
{
  dst->Data.AInt += src->Data.AInt;
  dst->Count += src->Count;
}

//{* end add method *}

THDataRec::THDataRec(const string Name, THDataType dt, THDataMode dm, THCollectMode cm, THParticleSize ps, bool WriteToDisk)
{
  DevName = Name;
  FWriteToDisk = WriteToDisk;
  // settings
  DataType = dt;
  DataMode = dm;
  CollectMode = cm;
  ParticleSize = ps;
  // history
  SDate = 0;
  SIndex = 0;
  SCount = 0;
  for (int i = 0; i < H_SECONDS; i++) memset(&SData[i], 0, sizeof(THData));
  MDate = 0;
  MIndex = 0;
  MCount = 0;
  for (int i = 0; i < H_MINUTES; i++) memset(&MData[i], 0, sizeof(THData));
  HDate = 0;
  HIndex = 0;
  HCount = 0;
  for (int i = 0; i < H_HOURS; i++) memset(&HData[i], 0, sizeof(THData));
  DDate = 0;
  DIndex = 0;
  DCount = 0;
  for(int i = 0; i < H_DAYS; i++) memset(&DData[i], 0, sizeof(THData));
  // current
  memset(&LData, 0, sizeof(THData));
  LDate = 0;
  // buffer
  BCount = 0;
  WDate = GetTimestamp();
  // add method
  switch (dm)
  {
    case dmByte:
    {
	  switch (cm)
	  {
	    case cmAverage:
	    {
	    	AddMethod = &AddBA;
	    	break;
	    }
	    case cmReplace:
	    {
	    	AddMethod = &AddBR;
	    	break;
	    }
	    case cmMax:
	    {
	    	AddMethod = &AddBM;
	    	break;
	    }
	    case cmMin:
	    {
	    	AddMethod = &AddBS;
	    	break;
	    }
	    case cmTotal:
	    {
	    	AddMethod = &AddBT;
	    	break;
	    }
	  }
	  break;
    }
    case dmWord:
    {
      switch (cm)
      {
        case cmAverage:
        {
        	AddMethod = &AddWA;
        	break;
        }
        case cmReplace:
        {
        	AddMethod = &AddWR;
        	break;
        }
        case cmMax:
        {
        	AddMethod = &AddWM;
        	break;
        }
        case cmMin:
        {
        	AddMethod = &AddWS;
        	break;
        }
        case cmTotal:
        {
        	AddMethod = &AddWT;
        	break;
        }
      }
      break;
    }
    case dmCardinal:
    {
      switch(cm)
      {
        case cmAverage:
        {
        	AddMethod = &AddCA;
        	break;
        }
        case cmReplace:
        {
        	AddMethod = &AddCR;
        	break;
        }
        case cmMax:
        {
        	AddMethod = &AddCM;
        	break;
        }
        case cmMin:
        {
        	AddMethod = &AddCS;
        	break;
        }
        case cmTotal:
        {
        	AddMethod = &AddCT;
        	break;
        }
      }
      break;
    }
    case dmDouble:
    {
      switch (cm)
      {
    	case cmAverage:
        {
        	AddMethod = &AddDA;
        	break;
        }
    	case cmReplace:
    	{
    		AddMethod = &AddDR;
    		break;
    	}
    	case cmMax:
    	{
    		AddMethod = &AddDM;
    		break;
    	}
    	case cmMin:
    	{
    		AddMethod = &AddDS;
    		break;
    	}
    	case cmTotal:
    	{
    		AddMethod = &AddDT;
    		break;
    	}
      }
      break;
    }
    case dmInt64:
    {
      switch(cm)
      {
        case cmAverage:
        {
        	AddMethod = &AddIA;
        	break;
        }
        case cmReplace:
        {
        	AddMethod = &AddIR;
        	break;
        }
        case cmMax:
        {
        	AddMethod = &AddIM;
        	break;
        }
        case cmMin:
        {
        	AddMethod = &AddIS;
        	break;
        }
        case cmTotal:
        {
        	AddMethod = &AddIT;
        	break;
        }
      }
      break;
    }
    case dmFloat:
    {
      switch (cm)
      {
        case cmAverage:
        {
        	AddMethod = &AddSA;
        	break;
        }
        case cmReplace:
        {
        	AddMethod = &AddSR;
        	break;
        }
        case cmMax:
        {
        	AddMethod = &AddSM;
        	break;
        }
        case cmMin:
        {
        	AddMethod = &AddSS;
        	break;
        }
        case cmTotal:
        {
        	AddMethod = &AddST;
        	break;
        }
      }
      break;
    }

    case dmInt:
    {
      switch(cm)
      {
        case cmAverage:
        {
       	  AddMethod = &AddiA;
          break;
        }
        case cmReplace:
        {
          AddMethod = &AddiR;
          break;
        }
        case cmMax:
        {
          AddMethod = &AddiM;
          break;
        }
        case cmMin:
        {
          AddMethod = &AddiS;
          break;
        }
        case cmTotal:
        {
          AddMethod = &AddiT;
          break;
        }
      }
      break;
    }
  }
}

void THDataRec::Get(THParticleSize ps, int64_t StartDate, int64_t EndDate, PHDataArray data)
{
  // 1. format
  int64_t d1 = StartDate;
  int64_t d2 = EndDate;
  int64_t s1, s2, s;
  int p, mp, lp, rp, tsp, tep, sp, ep;
  if (d1 > d2)
  {
    s = d1;
    d1 = d2;
    d2 = s;
  }
  // 2. search
  switch (ps)
  {
    case psMinute:
    {
      if (!MCount)
      {
        data->Count = 0;
        return;
      }
      ep = MIndex + H_MINUTES;
      sp = ep - MCount + 1;
      s2 = FormatTimeStamp(ps, MDates[ep % H_MINUTES]);
      s1 = FormatTimeStamp(ps, MDates[sp % H_MINUTES]);
      if ((d1 > s2) || (d2 < s1))
      {
        data->Count = 0;
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_MINUTES;
        if (MDates[p] > d1)
          if (tep == mp)
        	break;
          else
        	tep = mp;
        else if (MDates[p] < d1)
          if (tsp == mp)
        	break;
          else
           tsp = mp;
        else
        {
           lp = mp;
           break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
    	if ((tsp + 1) == tep)
    	  lp = tep;
    	else
    	  lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_MINUTES;
        if (MDates[p] > d2)
          if (tep == mp)
            break;
          else
        	tep = mp;
        else if (MDates[p] < d2)
          if (tsp == mp)
        	break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 fetch
      data->Count = rp - lp + 1;
      for (mp = lp; mp <= rp; mp++)
      {
        memcpy(&data->Data[mp - lp], &MData[mp % H_MINUTES], sizeof(THData));
        memcpy(&data->Dates[mp - lp], &MDates[mp % H_MINUTES], sizeof(int64_t));
      }
      break;
    }
    case psHour:
    {
      if (HCount == 0)
      {
        data->Count = 0;
        return;
      }
      ep = HIndex + H_HOURS;
      sp = ep - HCount + 1;
      s2 = FormatTimeStamp(ps, HDates[ep % H_HOURS]);
      s1 = FormatTimeStamp(ps, HDates[sp % H_HOURS]);
      if ((d1 > s2) || (d2 < s1))
      {
    	data->Count = 0;
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_HOURS;
        if (HDates[p] > d1)
          if (tep == mp)
        	break;
          else
            tep = mp;
        else if (HDates[p] < d1)
          if (tsp == mp)
        	break;
          else
        	tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
    	if ((tsp + 1) == tep)
    	  lp = tep;
    	else
    	  lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_HOURS;
        if (HDates[p] > d2)
          if (tep == mp)
        	break;
          else
        	tep = mp;
        else if (HDates[p] < d2)
         if (tsp == mp)
           break;
         else
           tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 fetch
      data->Count = rp - lp + 1;
      for (mp = lp; mp <= rp; mp++)
      {
        memcpy(&data->Data[mp - lp], &HData[mp % H_HOURS], sizeof(THData));
        memcpy(&data->Dates[mp - lp], &HDates[mp % H_HOURS], sizeof(int64_t));
      }
      break;
    }
    case psDay:
    {
      if (!DCount)
      {
        data->Count = 0;
        return;
      }
      ep = DIndex + H_DAYS;
      sp = ep - DCount + 1;
      s2 = FormatTimeStamp(ps, DDates[ep % H_DAYS]);
      s1 = FormatTimeStamp(ps, DDates[sp % H_DAYS]);
      if ((d1 > s2) || (d2 < s1))
      {
    	data->Count = 0;
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_DAYS;
        if (DDates[p] > d1)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (DDates[p] < d1)
          if (tsp == mp)
            break;
          else
        	tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
        if ((tsp + 1) == tep)
          lp = tep;
        else
          lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_DAYS;
        if (DDates[p] > d2)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (DDates[p] < d2)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 fetch
      data->Count = rp - lp + 1;
      for(mp = lp; mp <= rp; mp++)
      {
        memcpy(&data->Data[mp - lp], &DData[mp % H_DAYS], sizeof(THData));
        memcpy(&data->Dates[mp - lp], &DDates[mp % H_DAYS], sizeof(int64_t));
      }
      break;
    }
    default:
    {
      if (SCount == 0)
      {
        data->Count = 0;
        return;
      }
      ep = SIndex + H_SECONDS;
      sp = ep - SCount + 1;
      s2 = FormatTimeStamp(ps, SDates[ep % H_SECONDS]);
      s1 = FormatTimeStamp(ps, SDates[sp % H_SECONDS]);
      if ((d1 > s2) || (d2 < s1))
      {
        data->Count = 0;
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_SECONDS;
        if (SDates[p] > d1)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (SDates[p] < d1)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
        if ((tsp + 1) == tep)
          lp = tep;
        else
          lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_SECONDS;
        if (SDates[p] > d2)
          if (tep == mp)
            break;
          else
        	tep = mp;
        else if (SDates[p] < d2)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 fetch
      data->Count = rp - lp + 1;
      for (mp = lp; mp <= rp; mp++)
      {
    	memcpy(&data->Data[mp - lp], &SData[mp % H_SECONDS], sizeof(THData));
    	memcpy(&data->Dates[mp - lp], &SDates[mp % H_SECONDS], sizeof(int64_t));
      }
    }
  }
}

Int64 THDataRec::FormatTimeStamp(THParticleSize ps, Int64 d)
{
  if (ps == psDefault) ps = ParticleSize;
  switch (ps)
  {
    case psMinute: return d - (d % 60);
    case psHour: return d - (d % 3600);
    case psDay: return d - (d % 86400);
    default: return d;
  }
}

void THDataRec::IncTimestamp(pInt64 d, THParticleSize ps)
{
  if (ps == psDefault) ps = ParticleSize;
  switch (ps)
  {
    case psMinute:
    {
    	(*d) += 60;
    	break;
    }
    case psHour:
    {
    	(*d) += 3600;
    	break;
    }
    case psDay:
    {
    	(*d) += 86400;
    	break;
    }
    default: (*d)++;
  }
}

void THDataRec::GetF(THParticleSize ps, Int64 StartDate, Int64 EndDate, PHDataArray data, PHData min)
{
  Int64 d1, d2, s, s1, s2, ms;
  int p, mp, lp, rp, tsp, tep, sp, ep;
  double pp;
  // 1. format
  d1 = StartDate;
  d2 = EndDate;
  if (d1 > d2)
  {
    s = d1;
    d1 = d2;
    d2 = s;
  }
  // 1.2 check distance 0
  Int64 dis = d2 - d1;
  Int64 nstep = 0;
  switch(ps)
  {
  	  case psMinute:
  	  {
  		  nstep = 60;
  		  break;
  	  }
  	  case psHour:
  	  {
  		  nstep = 3600;
  	  	  break;
  	  }
  	  case psDay:
  	  {
  		  nstep = 86400;
  		  break;
  	  }
  	  default: nstep = 1;
  }
  bool fix_flag = false;
  if (dis < nstep)
  {
	  d2 = d1 + nstep;
	  fix_flag = true;
  }
  // 1.5 resize
  d1 = FormatTimeStamp(ps, d1);
  d2 = FormatTimeStamp(ps, d2);
  // 2. search
  switch (ps)
  {
    case psMinute:
    {
      if (!MCount)
      {
        data->Count = (d2 / 60) - (d1 / 60) + 1;
        s = d1;
        for(p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 60;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      ep = MIndex + H_MINUTES;
      sp = ep - MCount + 1;
      s2 = FormatTimeStamp(ps, MDates[ep % H_MINUTES]);
      s1 = FormatTimeStamp(ps, MDates[sp % H_MINUTES]);
      if ((d1 > s2) || (d2 < s1))
      {
        data->Count = (d2 / 60) - (d1 / 60) + 1;
        s = d1;
        for (p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 60;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_MINUTES;
        if (MDates[p] > d1)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (MDates[p] < d1)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
        if ((tsp + 1) == tep)
          lp = tep;
        else
          lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_MINUTES;
        if (MDates[p] > d2)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (MDates[p] < d2)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 generate x-axis data & fetch
      data->Count = (d2 / 60) - (d1 / 60) + 1;
      ms = d1;
      p = lp;
      pp = 99999999;
      s = 99999999;
      mp = 0;
      while (ms <= d2)
      {
        data->Dates[mp] = ms;
        if (Compare(MDates[p % H_MINUTES], ms, ps))
        {
          memcpy(&(data->Data[mp]), &MData[p % H_MINUTES], sizeof(THData));
          if (s > data->Data[mp].Count) s = data->Data[mp].Count;
          switch (DataMode)
          {
            case dmByte:
            {
            	if (pp > data->Data[mp].Data.AByte) pp = data->Data[mp].Data.AByte;
            	break;
            }
            case dmWord:
            {
            	if (pp > data->Data[mp].Data.AWord) pp = data->Data[mp].Data.AWord;
            	break;
            }
            case dmCardinal:
            {
            	if (pp > data->Data[mp].Data.ADWord) pp = data->Data[mp].Data.ADWord;
            	break;
            }
            case dmDouble:
            {
            	if (pp > data->Data[mp].Data.ADouble) pp = data->Data[mp].Data.ADouble;
            	break;
            }
            case dmInt64:
            {
            	if (pp > data->Data[mp].Data.AInt64) pp = data->Data[mp].Data.AInt64;
            	break;
            }
            case dmFloat:
            {
            	if (pp > data->Data[mp].Data.AFloat) pp = data->Data[mp].Data.AFloat;
            	break;
            }
            case dmInt:
            {
              	if (pp > data->Data[mp].Data.AInt) pp = data->Data[mp].Data.AInt;
               	break;
            }
          }
          p++;
        }
        else
          memset(&data->Data[mp], 0, sizeof(THData));
        IncTimestamp(&ms, ps);
        mp++;
      }
      min->Count = s;
      switch (DataMode)
      {
        case dmByte:
        {
        	min->Data.AByte = pp + 0.1;
        	break;
        }
        case dmWord:
        {
        	min->Data.AWord = pp + 0.1;
        	break;
        }
        case dmCardinal:
        {
        	min->Data.ADWord = pp + 0.1;
        	break;
        }
        case dmDouble:
        {
        	min->Data.ADouble = pp;
        	break;
        }
        case dmInt64:
        {
        	min->Data.AInt64 = pp + 0.1;
        	break;
        }
        case dmFloat:
        {
        	min->Data.AFloat = pp;
        	break;
        }
        case dmInt:
        {
          	min->Data.AInt = pp + 0.1;
          	break;
        }
      }
      break;
    }
    case psHour:
    {
      if (!HCount)
      {
        data->Count = (d2 / 3600) - (d1 / 3600) + 1;
        s = d1;
        for (p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 3600;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      ep = HIndex + H_HOURS;
      sp = ep - HCount + 1;
      s2 = FormatTimeStamp(ps, HDates[ep % H_HOURS]);
      s1 = FormatTimeStamp(ps, HDates[sp % H_HOURS]);
      if ((d1 > s2) || (d2 < s1))
      {
        data->Count = (d2 / 3600) - (d1 / 3600) + 1;
        s = d1;
        for (p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 3600;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_HOURS;
        if (HDates[p] > d1)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (HDates[p] < d1)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
        if ((tsp + 1) == tep)
          lp = tep;
        else
          lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_HOURS;
        if (HDates[p] > d2)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (HDates[p] < d2)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 generate x-axis data & fetch
      data->Count = (d2 / 3600) - (d1 / 3600) + 1;
      ms = d1;
      p = lp;
      pp = 99999999;
      s = 99999999;
      mp = 0;
      while (ms <= d2)
      {
        data->Dates[mp] = ms;
        if (Compare(HDates[p % H_HOURS], ms, ps))
        {
          memcpy(&data->Data[mp], &HData[p % H_HOURS], sizeof(THData));
          if (s > data->Data[mp].Count) s = data->Data[mp].Count;
          switch (DataMode)
          {
            case dmByte:
            {
            	if (pp > data->Data[mp].Data.AByte) pp = data->Data[mp].Data.AByte;
            	break;
            }
            case dmWord:
            {
            	if (pp > data->Data[mp].Data.AWord) pp = data->Data[mp].Data.AWord;
            	break;
            }
            case dmCardinal:
            {
            	if (pp > data->Data[mp].Data.ADWord) pp = data->Data[mp].Data.ADWord;
            	break;
            }
            case dmDouble:
            {
            	if (pp > data->Data[mp].Data.ADouble) pp = data->Data[mp].Data.ADouble;
            	break;
            }
            case dmInt64:
            {
            	if (pp > data->Data[mp].Data.AInt64) pp = data->Data[mp].Data.AInt64;
            	break;
            }
            case dmFloat:
            {
            	if (pp > data->Data[mp].Data.AFloat) pp = data->Data[mp].Data.AFloat;
            	break;
            }
            case dmInt:
            {
            	if (pp > data->Data[mp].Data.AInt) pp = data->Data[mp].Data.AInt;
            	break;
            }
          }
          p++;
        }
        else
          memset(&data->Data[mp], 0, sizeof(THData));
        IncTimestamp(&ms, ps);
        mp++;
      }
      min->Count = s;
      switch (DataMode)
      {
        case dmByte:
        {
        	min->Data.AByte = pp + 0.1;
        	break;
        }
        case dmWord:
        {
        	min->Data.AWord = pp + 0.1;
        	break;
        }
        case dmCardinal:
        {
        	min->Data.ADWord = pp + 0.1;
        	break;
        }
        case dmDouble:
        {
        	min->Data.ADouble = pp;
        	break;
        }
        case dmInt64:
        {
        	min->Data.AInt64 = pp + 0.1;
        	break;
        }
        case dmFloat:
        {
        	min->Data.AFloat = pp;
        	break;
        }
        case dmInt:
        {
        	min->Data.AInt = pp + 0.1;
        	break;
        }
      }
      break;
    }
    case psDay:
    {
      if (!DCount)
      {
        data->Count = d2 / 86400 - d1 / 86400 + 1;
        s = d1;
        for(p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 86400;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      ep = DIndex + H_DAYS;
      sp = ep - DCount + 1;
      s2 = FormatTimeStamp(ps, DDates[ep % H_DAYS]);
      s1 = FormatTimeStamp(ps, DDates[sp % H_DAYS]);
      if ((d1 > s2) || (d2 < s1))
      {
    	data->Count = d2 / 86400 - d1 / 86400 + 1;
        s = d1;
        for (p = 0; p < data->Count; p++)
        {
          memset(&data->Data[p], 0, sizeof(THData));
          data->Dates[p] = s;
          s += 86400;
        }
        memset(min, 0, sizeof(THData));
        return;
      }
      // 2.1 search left
      tsp = sp;
      tep = ep;
      lp = -1;
      rp = -1;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_DAYS;
        if (DDates[p] > d1)
          if (tep == mp)
            break;
          else
           tep = mp;
        else if (DDates[p] < d1)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          lp = mp;
          break;
        }
      } while (true);
      // 2.2 fix
      if (lp == -1)
      {
        if ((tsp + 1) == tep)
          lp = tep;
        else
          lp = sp;
      }
      // 2.3 search right
      tsp = lp;
      tep = ep;
      do {
        mp = (tsp + tep) >> 1;
        p = mp % H_DAYS;
        if (DDates[p] > d2)
          if (tep == mp)
            break;
          else
            tep = mp;
        else if (DDates[p] < d2)
          if (tsp == mp)
            break;
          else
            tsp = mp;
        else
        {
          rp = mp;
          break;
        }
      } while (true);
      // 2.4 fix
      if (rp == -1)
      {
        if ((tsp + 1) == tep)
          rp = tsp;
        else
          rp = ep;
      }
      // 2.5 generate x-axis data & fetch
      data->Count = (d2 / 86400) - (d1 / 86400) + 1;
      ms = d1;
      p = lp;
      pp = 99999999;
      s = 99999999;
      mp = 0;
      while (ms <= d2)
      {
        data->Dates[mp] = ms;
        if (Compare(DDates[p % H_DAYS], ms, ps))
        {
          memcpy(&data->Data[mp], &DData[p % H_DAYS], sizeof(THData));
          if (s > data->Data[mp].Count) s = data->Data[mp].Count;
          switch (DataMode)
          {
            case dmByte:
            {
            	if (pp > data->Data[mp].Data.AByte) pp = data->Data[mp].Data.AByte;
            	break;
            }
            case dmWord:
            {
            	if (pp > data->Data[mp].Data.AWord) pp = data->Data[mp].Data.AWord;
            	break;
            }
            case dmCardinal:
            {
            	if (pp > data->Data[mp].Data.ADWord) pp = data->Data[mp].Data.ADWord;
            	break;
            }
            case dmDouble:
            {
            	if (pp > data->Data[mp].Data.ADouble) pp = data->Data[mp].Data.ADouble;
            	break;
            }
            case dmInt64:
            {
            	if (pp > data->Data[mp].Data.AInt64) pp = data->Data[mp].Data.AInt64;
            	break;
            }
            case dmFloat:
            {
            	if (pp > data->Data[mp].Data.AFloat) pp = data->Data[mp].Data.AFloat;
            	break;
            }
            case dmInt:
            {
            	if (pp > data->Data[mp].Data.AInt) pp = data->Data[mp].Data.AInt;
            	break;
            }
          }
          p++;
        }
        else
          memset(&data->Data[mp], 0, sizeof(THData));
        IncTimestamp(&ms, ps);
        mp++;
      }
      min->Count = s;
      switch (DataMode)
      {
        case dmByte:
        {
        	min->Data.AByte = pp + 0.1;
        	break;
        }
        case dmWord:
        {
        	min->Data.AWord = pp + 0.1;
        	break;
        }
        case dmCardinal:
        {
        	min->Data.ADWord = pp + 0.1;
        	break;
        }
        case dmDouble:
        {
        	min->Data.ADouble = pp;
        	break;
        }
        case dmInt64:
        {
        	min->Data.AInt64 = pp + 0.1;
        	break;
        }
        case dmFloat:
        {
        	min->Data.AFloat = pp;
        	break;
        }
        case dmInt:
        {
        	min->Data.AInt = pp + 0.1;
        	break;
        }
      }
      break;
    }

  default:
  {
    if (!SCount)
    {
      data->Count = d2 - d1 + 1;
      for(p = 0; p < data->Count; p++)
      {
        memset(&data->Data[p], 0, sizeof(THData));
        data->Dates[p] = d1 + p;
      }
      memset(min, 0, sizeof(THData));
      return;
    }
    ep = SIndex + H_SECONDS;
    sp = ep - SCount + 1;
    s2 = FormatTimeStamp(ps, SDates[ep % H_SECONDS]);
    s1 = FormatTimeStamp(ps, SDates[sp % H_SECONDS]);
    if ((d1 > s2) || (d2 < s1))
    {
      data->Count = d2 - d1 + 1;
      for (p = 0; p < data->Count; p++)
      {
        memset(&data->Data[p], 0, sizeof(THData));
        data->Dates[p] = d1 + p;
      }
      memset(min, 0, sizeof(THData));
      return;
    }
    // 2.1 search left
    tsp = sp;
    tep = ep;
    lp = -1;
    rp = -1;
    do {
      mp = (tsp + tep) >> 1;
      p = mp % H_SECONDS;
      if (SDates[p] > d1)
        if (tep == mp)
          break;
        else
          tep = mp;
      else if (SDates[p] < d1)
        if (tsp == mp)
          break;
        else
          tsp = mp;
      else
      {
        lp = mp;
        break;
      }
    } while (true);
    // 2.2 fix
    if (lp == -1)
    {
      if ((tsp + 1) == tep)
        lp = tep;
      else
        lp = sp;
    }
    // 2.3 search right
    tsp = lp;
    tep = ep;
    do {
      mp = (tsp + tep) >> 1;
      p = mp % H_SECONDS;
      if (SDates[p] > d2)
        if (tep == mp)
          break;
        else
          tep = mp;
      else if (SDates[p] < d2)
        if (tsp == mp)
          break;
        else
          tsp = mp;
      else
      {
        rp = mp;
        break;
      }
    } while (true);
    // 2.4 fix
    if (rp == -1)
    {
      if ((tsp + 1) == tep)
        rp = tsp;
      else
        rp = ep;
    }
    // 2.5 generate x-axis data & fetch
    data->Count = d2 - d1 + 1;
    ms = d1;
    p = lp;
    pp = 99999999;
    s = 99999999;
    mp = 0;
    while (ms <= d2)
    {
      data->Dates[mp] = ms;
      if (Compare(SDates[p % H_SECONDS], ms, ps))
      {
        memcpy(&data->Data[mp], &SData[p % H_SECONDS], sizeof(THData));
        if (s > data->Data[mp].Count) s = data->Data[mp].Count;
        switch (DataMode)
        {
          case dmByte:
          {
        	if (pp > data->Data[mp].Data.AByte) pp = data->Data[mp].Data.AByte;
        	break;
          }
          case dmWord:
          {
        	if (pp > data->Data[mp].Data.AWord) pp = data->Data[mp].Data.AWord;
        	break;
          }
          case dmCardinal:
          {
        	if (pp > data->Data[mp].Data.ADWord) pp = data->Data[mp].Data.ADWord;
        	break;
          }
          case dmDouble:
          {
        	if (pp > data->Data[mp].Data.ADouble) pp = data->Data[mp].Data.ADouble;
        	break;
          }
          case dmInt64:
          {
        	  if (pp > data->Data[mp].Data.AInt64) pp = data->Data[mp].Data.AInt64;
        	  break;
          }
          case dmFloat:
          {
        	  if (pp > data->Data[mp].Data.AFloat) pp = data->Data[mp].Data.AFloat;
        	  break;
          }
          case dmInt:
          {
        	  if (pp > data->Data[mp].Data.AInt) pp = data->Data[mp].Data.AInt;
        	  break;
          }
        }
        p++;
      }
      else
        memset(&data->Data[mp], 0, sizeof(THData));
      IncTimestamp(&ms, ps);
      mp++;
    }
    min->Count = s;
    switch (DataMode)
    {
      case dmByte:
      {
    	  min->Data.AByte = pp + 0.1;
    	  break;
      }
      case dmWord:
      {
    	  min->Data.AWord = pp + 0.1;
    	  break;
      }
      case dmCardinal:
      {
    	  min->Data.ADWord = pp + 0.1;
    	  break;
      }
      case dmDouble:
      {
    	  min->Data.ADouble = pp;
    	  break;
      }
      case dmInt64:
      {
    	  min->Data.AInt64 = pp + 0.1;
    	  break;
      }
      case dmFloat:
      {
    	  min->Data.AFloat = pp;
    	  break;
      }
      case dmInt:
      {
    	  min->Data.AInt = pp + 0.1;
    	  break;
      }
    }
  }
}
  // 100 check fix
  if (fix_flag && (data->Count == 2) && (data->Data[1].Count == 0)) data->Count = 1;
}

bool THDataRec::Compare(Int64 d1, Int64 d2, THParticleSize ps)
{
  if (ps == psDefault) ps = ParticleSize;
  switch (ps)
  {
    case psMinute: return (d1 / 60) == (d2 / 60);
    case psHour: return (d1 / 3600) == (d2 / 3600);
    case psDay: return (d1 / 86400) == (d2 / 86400);
    default: return d1 == d2;
  }
}

void THDataRec::Add(PHData data, const Int64 LastDate, THParticleSize ps)
{
  if (ps == psDefault) ps = ParticleSize;
  do {
    switch (ps)
    {
      case psMinute:  // by minute
      {
        if (Compare(MDate, LastDate, psMinute))  // date is same, need complex add
          (*AddMethod)(data, &MData[MIndex]);
        else
        {
          // add new record
          if (MCount < H_MINUTES) MCount++;
          //memcpy(&MData[MIndex], data, sizeof(THData));
          if (MDate)
            MIndex = (MIndex + 1) % H_MINUTES;
          MDates[MIndex] = LastDate;

          //memset(&MData[MIndex], 0, sizeof(THData));
          memcpy(&MData[MIndex], data, sizeof(THData));
          MDate = LastDate;
        }
        // do statistics and trans to uplevel
        ps = psHour;
        break;
      }
      case psHour:  // by hour
      {
        if (Compare(HDate, LastDate, psHour)) // date is same, need complex add
          (*AddMethod)(data, &HData[HIndex]);
        else
        {
          // add new record
          if (HCount < H_HOURS) HCount++;
          //memcpy(&HData[HIndex], data, sizeof(THData));
          if (HDate)
            HIndex = (HIndex + 1) % H_HOURS;
          HDates[HIndex] = LastDate;

          //memset(&HData[HIndex], 0, sizeof(THData));
          memcpy(&HData[HIndex], data, sizeof(THData));
          HDate = LastDate;
        }
        // do statistics and trans to uplevel
        ps = psDay;
        break;
      }
      case psDay: // by day
      {
        if (Compare(DDate, LastDate, psDay))  // date is same, need complex add
          (*AddMethod)(data, &DData[DIndex]);
        else
        {
          // add new record
          if (DCount < H_DAYS) DCount++;
          //memcpy(&DData[DIndex], data, sizeof(THData));
          if (DDate)
            DIndex = (DIndex + 1) % H_DAYS;
          DDates[DIndex] = LastDate;

          //memset(&DData[DIndex], 0, sizeof(THData));
          memcpy(&DData[DIndex], data, sizeof(THData));
          DDate = LastDate;
        }
        return;
      }
      default: // by second
      {
        if (Compare(SDate, LastDate, psSecond))  // date is same, need complex add
          (*AddMethod)(data, &SData[SIndex]);
        else
        {
          if (SCount < H_SECONDS) SCount++;
          //memcpy(&SData[SIndex], data, sizeof(THData));
          if (SDate)
            SIndex = (SIndex + 1) % H_SECONDS;
          SDates[SIndex] = LastDate;
          memcpy(&SData[SIndex], data, sizeof(THData));
          //memset(&SData[SIndex], 0, sizeof(THData));
          SDate = LastDate;
        }
        // do statistics and trans to uplevel
        ps = psMinute;
      }
    }
  } while (true);
}

void THDataRec::Write()
{
  int i;
  // 1. check
  int64_t d = GetTimestamp();
  string fn = ThisPath();
  fn = fn + "working/" + DevName + ".dat";
  string pn = ExtractFilepath(fn);
  if (!DirectoryExists(pn)) ForceDirectories(pn);
  if (FileExists(fn))
  {
    if ((!Compare(d, WDate, psDay)) || (GetFileSize(fn) > H_DAT_VOLUME))
    {
      // 2. need archive
      string s = ThisPath();
      pn = FormatDateTime("yyyymmdd", UnixToDateTime(WDate));
      string t = FormatDateTime("hhnnss", UnixToDateTime(WDate));
      s = s + string("history/") + pn + string("/")
        + DevName + string("-") + t + string(".dat");
      pn = ExtractFilepath(s);
      if (!DirectoryExists(pn)) ForceDirectories(pn);
      RenameFile(fn, s);
    };
  }
  if (!BCount) return;

  FILE * f = fopen(fn.c_str(), "wb+");
  if (f)
  {
    fseek(f, 0, SEEK_END);
    for(i = 0; i < BCount; i++)
    {
      fwrite(&BDates[i], 8, 1, f);
      switch (DataMode)
      {
        case dmByte:
        {
          fwrite(&BData[i].Data.AByte, 1, 1, f);
          break;
        }
        case dmWord:
        {
          fwrite(&BData[i].Data.AWord, 2, 1, f);
          break;
        }
        case dmCardinal:
        {
          fwrite(&BData[i].Data.ADWord, 4, 1, f);
          break;
        }
        case dmDouble:
        {
          fwrite(&BData[i].Data.ADouble, 8, 1, f);
          break;
        }
        case dmInt64:
        {
          fwrite(&BData[i].Data.AInt64, 8, 1, f);
          break;
        }
        case dmFloat:
        {
          fwrite(&BData[i].Data.AFloat, 4, 1, f);
          break;
        }
        case dmInt:
        {
          fwrite(&BData[i].Data.AInt, 4, 1, f);
          break;
        }
      }

    }
    fclose(f);
  }
  WDate = d;
  BCount = 0;
}

void THDataRec::Put(PHData data, Int64 time)
{
  // 1. format
  if (!time) time = GetTimestamp();
  // 2. history
  //WriteLn("Put data: ", data->Count, " ", data->AWord);
  Add(data, time);
  // 2. current
  LDate = time;
  memcpy(&LData, data, sizeof(THData));
  // 4. output
  if (!FWriteToDisk) return;
  memcpy(&BData[BCount], data, sizeof(THData));
  BDates[BCount++] = time;
  if ((BCount == H_BUFFERS) || ((time - BDates[0]) > 3)) Write();
}

THDataRec::~THDataRec()
{
  if (FWriteToDisk) Write();
}

bool CleanAndCompress()
{
  pInt64 ts = 0;
  int c, d;
  TStringList2 t;
  bool ret;
  do {
    #ifdef H_DEBUG
    printf("CleanAndCompress..\n");
    #endif
    c = 0;
    // check disk space
    d = -1;
    string tp = ThisPath();
    string hp = tp + "history/";
    while (DiskFree(tp) < (uint64_t)H_FREE_LIMIT)
    {
      t.clear();
      Dirs(hp, "*", false, t, ts);
      t.sort();
      if ((t.size() == 0) || ((int64_t)t.size() == d)) break;
      d = t.size();
      if (d > 0)
      {
    	TStringList2::iterator ti = t.begin();
        #ifdef H_DEBUG
        printf("For more space, delete dir %s\n", ti->c_str());
        #endif
        string cmd = string("rm ") + *ti + " -fR";
        system(cmd.c_str());
        c++;
      }
    }
    d = -1;
    string wp = tp + "working/";
    while (DiskFree(tp) < (uint64_t)H_FREE_LIMIT)
    {
      t.clear();
      DirFiles(wp, string("*"), true, t, ts);
      t.sort();
      if ((t.size() == 0) || ((int64_t)t.size() == d)) break;
      d = t.size();
      if (d > 0)
      {
    	TStringList2::iterator ti = t.begin();
	    #ifdef H_DEBUG
        printf("For more space, delete file %s\n", ti->c_str());
	    #endif
        DeleteFile(*ti);
        c++;
      }
    }
    d = -1;
    while (DiskFree(tp) > (uint64_t)H_FREE_LIMIT)
    {
      t.clear();
      DirFiles(hp, string("*.dat"), true, t, ts);
      t.sort();
      if ((t.size() == 0) || ((int64_t)t.size() == d)) break;
      d = t.size();
      if (d > 0)
      {
    	TStringList2::iterator ti = t.begin();
	    #ifdef H_DEBUG
        printf("For more space, compress file %s\n", ti->c_str());
	    #endif
        if (CompressAFile(*ti, true)) c++;
      }
    }
    ret = c > 0;
    #ifdef H_DEBUG
    printf("CleanAndCompress end.\n");
    #endif
    return ret;
  } while (true);
  return false;
}

#endif /* HDATAREC_CPP_ */
