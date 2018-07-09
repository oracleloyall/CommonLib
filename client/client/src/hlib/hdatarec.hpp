/*
 * hdatarec.hpp
 *
 *  Created on: 2013-9-21
 *      Author: root
 */

#ifndef HDATAREC_HPP_
#define HDATAREC_HPP_

#include "global.hpp"
#include "hpasutils.hpp"
#include "hdateutils.hpp"
#include "hzlib.hpp"

const int H_SECONDS = 300;
const int H_MINUTES = 480;
const int H_HOURS = 720;
const int H_DAYS = 730;
const int H_MAX = 730;

const int H_BUFFERS = 256;
const int H_DAT_VOLUME = 1024 * 256;
const int H_FREE_LIMIT = 1048576 * 3;

typedef struct THData
{
  int Count;
  union {
	unsigned char AByte;
	unsigned short int AWord;
	uint ADWord;
	double ADouble;
	uint64_t AInt64;
	float AFloat;
	int AInt;
  } Data;
} *PHData;

typedef struct THDataArray
{
  int Count;
  THData Data[H_MAX];
  uint64_t Dates[H_MAX];
} *PHDataArray;

enum THCollectMode {cmAverage, cmReplace, cmMax, cmMin, cmTotal};
enum THParticleSize {psDefault, psSecond, psMinute, psHour, psDay};
enum THDataMode {dmByte, dmWord, dmCardinal, dmDouble, dmInt64, dmFloat, dmInt};
enum THDataType {dtTemperature, dtHumidity, dtDew, dtVoltage, dtGalvano,
               dtPower, dtKiloWatt, dtSwitch, dtSpeed, dtMicroSeconds, dtBytes,
               dtUnknown};

typedef void (*THAddMethod)(PHData src, PHData dst);

void AddBA(PHData src, PHData dst);
void AddBR(PHData src, PHData dst);
void AddBM(PHData src, PHData dst);
void AddBS(PHData src, PHData dst);
void AddBT(PHData src, PHData dst);
void AddWA(PHData src, PHData dst);
void AddWR(PHData src, PHData dst);
void AddWM(PHData src, PHData dst);
void AddWS(PHData src, PHData dst);
void AddWT(PHData src, PHData dst);
void AddCA(PHData src, PHData dst);
void AddCR(PHData src, PHData dst);
void AddCM(PHData src, PHData dst);
void AddCS(PHData src, PHData dst);
void AddCT(PHData src, PHData dst);
void AddDA(PHData src, PHData dst);
void AddDR(PHData src, PHData dst);
void AddDM(PHData src, PHData dst);
void AddDS(PHData src, PHData dst);
void AddDT(PHData src, PHData dst);
void AddIA(PHData src, PHData dst);
void AddIR(PHData src, PHData dst);
void AddIM(PHData src, PHData dst);
void AddIS(PHData src, PHData dst);
void AddIT(PHData src, PHData dst);
void AddSA(PHData src, PHData dst);
void AddSR(PHData src, PHData dst);
void AddSM(PHData src, PHData dst);
void AddSS(PHData src, PHData dst);
void AddST(PHData src, PHData dst);
void AddiA(PHData src, PHData dst);
void AddiR(PHData src, PHData dst);
void AddiM(PHData src, PHData dst);
void AddiS(PHData src, PHData dst);
void AddiT(PHData src, PHData dst);

class THDataRec
{
  public:
    string DevName;
    bool FWriteToDisk;
    // settings
    THDataType DataType;
    THDataMode DataMode;
    THCollectMode CollectMode;
    THParticleSize ParticleSize;
    // history
    int64_t SDate;
    int SIndex;
    int SCount;
    int64_t SDates[H_SECONDS];
    THData SData[H_SECONDS];
    int64_t MDate;
    int MIndex;
    int MCount;
    int64_t MDates[H_MINUTES];
    THData MData[H_MINUTES];
    int64_t HDate;
    int HIndex;
    int HCount;
    int64_t HDates[H_HOURS];
    THData HData[H_HOURS];
    int64_t DDate;
    int DIndex;
    int DCount;
    int64_t DDates[H_DAYS];
    THData DData[H_DAYS];
    // buffer
    THData BData[H_BUFFERS];
    int64_t BDates[H_BUFFERS];
    int BCount;
    int64_t WDate;
    // current
    THData LData;
    int64_t LDate;
    // methods
    THAddMethod AddMethod;
    void Add(PHData data, const int64_t LastDate, THParticleSize ps = psDefault);
    bool Compare(int64_t d1, int64_t d2, THParticleSize ps = psDefault);
    int64_t FormatTimeStamp(THParticleSize ps, int64_t d);
    void IncTimestamp(int64_t* d, THParticleSize ps);
    void Write();
    void Get(THParticleSize ps, int64_t StartDate, int64_t EndDate, PHDataArray data);
    void GetF(THParticleSize ps, int64_t StartDate, int64_t EndDate, PHDataArray data, PHData min);
    void Put(PHData data, int64_t time = 0);
    THDataRec(const string Name, THDataType dt, THDataMode dm, THCollectMode cm, THParticleSize ps, bool WriteToDisk = true);
    ~THDataRec();
};

#endif /* HDATAREC_HPP_ */
