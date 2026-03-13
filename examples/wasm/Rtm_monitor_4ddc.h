/* This file was automatically generated from rmtld3synth tool version
v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14. */

  #ifndef RTM_MONITOR_4DDC_H
  #define RTM_MONITOR_4DDC_H

#include "../../rtmlib/src/reader.h"
#include "../../rtmlib/src/periodicmonitor.h"
#include "../../rtmlib/src/rmtld3/reader.h"
  #include "Rtm_compute_4ddc.h"

  /* 100, Event< std::underlying_type<_auto_gen_prop>::type > */
  #define RTM_MONITOR_4DDC_BUFFER_SIZE 100
  #define RTM_MONITOR_4DDC_BUFFER_TYPE Event< std::underlying_type<_auto_gen_prop>::type >
  #define RTML_BUFFER0_SETUP() \
    RTML_buffer<RTM_MONITOR_4DDC_BUFFER_TYPE, RTM_MONITOR_4DDC_BUFFER_SIZE> __buffer_rtm_monitor_4ddc;
    #define RTML_BUFFER0_TRIGGER_PERIODIC_MONITORS() \
RMTLD3_reader< \
RTML_reader<RTML_buffer<RTM_MONITOR_4DDC_BUFFER_TYPE, RTM_MONITOR_4DDC_BUFFER_SIZE>>> \
__trace_rtm_monitor_4ddc_0 = RMTLD3_reader< \
    RTML_reader<RTML_buffer<RTM_MONITOR_4DDC_BUFFER_TYPE, RTM_MONITOR_4DDC_BUFFER_SIZE>>>(__buffer_rtm_monitor_4ddc); \
\
Rtm_monitor_4ddc_0<RMTLD3_reader< \
        RTML_reader<RTML_buffer<RTM_MONITOR_4DDC_BUFFER_TYPE, RTM_MONITOR_4DDC_BUFFER_SIZE>>>> \
        rtm_mon0(200000, __trace_rtm_monitor_4ddc_0); \


  template<class T>
  class Rtm_monitor_4ddc_0 : public RTML_monitor<'4','d','d','c'> {

  private:
    T &trace;

  protected:
    three_valued_type _out = T_UNKNOWN;
    timespan tbegin;

    void run(){

      trace.synchronize();
      if (trace.set(tbegin) == 0) {
                    _out = _rtm_compute_4ddc_0<T>(trace,tbegin);
      }
      DEBUG_RMTLD3("status=%d, tbegin=%lu\n", _out, tbegin);
    }

  public:
    Rtm_monitor_4ddc_0(useconds_t p, T& trc, timespan& t): RTML_monitor(p,DEFAULT_SCHED,DEFAULT_PRIORITY), trace(trc), tbegin(t) { }
    Rtm_monitor_4ddc_0(useconds_t p, T& trc): RTML_monitor(p,DEFAULT_SCHED,DEFAULT_PRIORITY), trace(trc) { tbegin = clockgettime(); }
    Rtm_monitor_4ddc_0(useconds_t p, T& trc, int sche, int prio): RTML_monitor(p,sche,prio), trace(trc) { tbegin = clockgettime(); }
    Rtm_monitor_4ddc_0(useconds_t p, T& trc, int sche, int prio, timespan& t): RTML_monitor(p,sche,prio), trace(trc), tbegin(t) { }
   three_valued_type &getVeredict() { return _out; }
  };

  #endif //RTM_MONITOR_4DDC_H
