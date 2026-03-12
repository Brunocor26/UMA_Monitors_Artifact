/* This file was automatically generated from rmtld3synth tool version
  v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14. */

  #ifndef RTM_INSTRUMENT_4DDC_H_
  #define RTM_INSTRUMENT_4DDC_H_


#include "../rtmlib/src/writer.h"
#include "../rtmlib/src/rmtld3/rmtld3.h"

 #define RTM_INSTRUMENT_4DDC_BUFFER_SIZE 100

 class RTM_INSTRUMENT_4DDC{
 public:typedef enum _auto_gen_prop {b = 1, a = 2, } prop_t;

 typedef Event< std::underlying_type<_auto_gen_prop>::type > buffer_t;

static timespan time_of_s(timespan v) {
float _v = v; 
 _v /=  
1.; _v *= 1.; return _v; 
}
 

 
};

 template<typename T, T& buffer>
class Writer_RTM_INSTRUMENT_4DDC : public RTM_INSTRUMENT_4DDC {

  public:
 typename T::error_t push(std::underlying_type<_auto_gen_prop>::type s) {
      typename T::event_t e = typename T::event_t(s,0);
      return w.push(e);
    };

 typename T::error_t push(std::underlying_type<_auto_gen_prop>::type s, timespan t) {
      typename T::event_t e = typename T::event_t(s,t);
      return w.push_all(e);
    };

  private:
    RTML_writer<T> w = RTML_writer<T>(buffer);

};

// buffer will be assigned at ld step
extern RTML_buffer<Event<std::underlying_type<RTM_INSTRUMENT_4DDC::prop_t>::type>, RTM_INSTRUMENT_4DDC_BUFFER_SIZE> __buffer_rtm_monitor_4ddc;

using Writer_rtm__4ddc = Writer_RTM_INSTRUMENT_4DDC<RTML_buffer<Event<std::underlying_type<RTM_INSTRUMENT_4DDC::prop_t>::type>, RTM_INSTRUMENT_4DDC_BUFFER_SIZE>,__buffer_rtm_monitor_4ddc>;

     #endif //RTM_INSTRUMENT_4DDC_H_
