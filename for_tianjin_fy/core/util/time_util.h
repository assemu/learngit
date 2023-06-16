#ifndef CORE_UTIL_TIME_UTIL_H_
#define CORE_UTIL_TIME_UTIL_H_

#include <iostream>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/timer/timer.hpp>

using namespace std::chrono;

/**
 * 时间相关的工具函数
 */

//用boost实现的时间戳
inline int64_t get_timestamp()
{
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
  return (boost::posix_time::microsec_clock::universal_time() - epoch).total_milliseconds();
}

inline int64_t mSecsFromEpoch()
{
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
  return (boost::posix_time::microsec_clock::universal_time() - epoch).total_milliseconds();
}

//线程自选多少ms
inline void thread_spin_ms(int ms)
{
  boost::timer::cpu_timer t;
  while(t.elapsed().wall < ms * 1e6)
    ;
}

//计时器
class TimerClock
{
public:
  TimerClock()
  {
    update();
  }

  void update()
  {
    _start = high_resolution_clock::now();
  }

  //获取秒
  double getTimerSecond()
  {
    return getTimerMicroSec() * 0.000001;
  }

  //获取毫秒
  double getTimerMilliSec()
  {
    return getTimerMicroSec() * 0.001;
  }

  //获取微妙
  long long getTimerMicroSec()
  {
    //当前时钟减去开始时钟的count
    return duration_cast<microseconds>(high_resolution_clock::now() - _start).count();
  }

private:
  time_point<high_resolution_clock> _start;
};

//毫秒倒计时器
class CountDownTimer
{
public:
  CountDownTimer()
  {
    reset_ms(0);
  }

  //倒计时ms
  void reset_ms(double ms)
  {
    _ms = ms;
    _timer.update();
  }

  double msLeft()
  {
    double ts = _timer.getTimerMilliSec();
    return ts >= _ms ? 0 : _ms - ts;
  }

  bool hasTimeLeft()
  {
    return msLeft() > 0;
  }

private:
  double _ms;
  TimerClock _timer;
};

#endif //CORE_UTIL_TIME_UTIL_H_
