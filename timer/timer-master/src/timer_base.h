/**
 *   @file     timer_base.h
 *   @brief    ��ʱ������
 *   @author   kurtlau
 *   @date     2013-04-08
 */
#ifndef __TIMER_BASE_H__
#define __TIMER_BASE_H__

#include <sys/time.h>

class TimerBase
{
public:
  TimerBase(): m_expire(0), m_process_time(){};

  virtual ~TimerBase(){};

  virtual int Run() = 0;

  virtual int PreRun() = 0;

  virtual int PostRun() = 0;

  TimerBase& operator=(const TimerBase &timer);

  void SetProcessTime(const struct timeval &tv){ m_process_time = tv;};

  const struct timeval &GetProcessTime()const {return m_process_time;};

  void SetExpireTick(const unsigned int expire){m_expire = expire;};

  unsigned int GetExpireTick()const {return m_expire;};

  void SetIntervalTime(unsigned int interval){m_interval = interval;};
  
  unsigned int GetIntervalTime(){return m_interval;};

protected:
  unsigned int   m_expire;         // �����TimerWrap��Init�Ժ��tick��
  unsigned int   m_interval;		 // ����Ժ�ʱ
  struct timeval m_process_time;   // �����ʱ�䡣
};

#endif


