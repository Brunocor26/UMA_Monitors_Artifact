/* This file was automatically generated from rmtld3synth tool version
  v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14. */

  #ifndef RTM_INSTRUMENT_3DA2_H_
  #define RTM_INSTRUMENT_3DA2_H_


 #include <writer.h>
 #include <rmtld3/rmtld3.h>

 #define RTM_INSTRUMENT_3DA2_BUFFER_SIZE 100

 class RTM_INSTRUMENT_3DA2{
 public:typedef enum _auto_gen_prop {a = 1, } prop_t;

 typedef Event< std::underlying_type<_auto_gen_prop>::type > buffer_t;

static timespan time_of_s(timespan v) {
float _v = v; 
 _v /=  
1.; _v *= 1.; return _v; 
}
 

 
};

 template<typename T, T& buffer>
class Writer_RTM_INSTRUMENT_3DA2 : public RTM_INSTRUMENT_3DA2 {

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
extern RTML_buffer<Event<std::underlying_type<RTM_INSTRUMENT_3DA2::prop_t>::type>, RTM_INSTRUMENT_3DA2_BUFFER_SIZE> __buffer_rtm_monitor_3da2;

using Writer_rtm__3da2 = Writer_RTM_INSTRUMENT_3DA2<RTML_buffer<Event<std::underlying_type<RTM_INSTRUMENT_3DA2::prop_t>::type>, RTM_INSTRUMENT_3DA2_BUFFER_SIZE>,__buffer_rtm_monitor_3da2>;

     #endif //RTM_INSTRUMENT_3DA2_H_
