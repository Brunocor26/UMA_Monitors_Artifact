/* This file was automatically generated from rmtld3synth tool version
v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14.

Settings:
out_dir -> 'out'
rtm_buffer_size -> 100
rtm_event_subtype -> 'std::underlying_type<_auto_gen_prop>::type'
fm_num_duration -> 1
version -> 'v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14'
input_exp -> (5. < int[15.] (a))
rtm_monitor_name_prefix -> 'rtm_#_%'
rtm_monitor_time_unit -> 's'
fm_num_prop -> 1
rtm_period -> 200000
prop_map_reverse -> (1->a) 
rtm_min_inter_arrival_time -> 1
init -> true
unique_id_counter -> 1
rtm_max_period -> 2000000
rtm_event_type -> 'Event'
input_exp_dsl -> 'duration of a in 0 .. 15 >= 5'
prop_map -> (a->1) 
gen_tests -> 'false'

Formula(s):
- (5. < int[15.] (a))

*/

  #ifndef RTM_COMPUTE_3DA2_H_
  #define RTM_COMPUTE_3DA2_H_

  #include <rmtld3/rmtld3.h>
  #include <rmtld3/formulas.h>
  
  #define RTM_TIME_UNIT s
  
  // Propositions
  enum _auto_gen_prop {PROP_a = 1, }; 
  template <typename T> class Eval_duration3 {
  public:
    static three_valued_type eval_phi1(T &trace, timespan &t) {
      auto sf = prop<T>(trace, PROP_a, t);
      return sf;
    };
    static three_valued_type eval_phi2(T &trace, timespan &t) {
      return T_UNKNOWN;
    };
  };
        template<class T>
        three_valued_type _rtm_compute_3da2_0 (T &trace, timespan &t) {
          return [](T &trace, timespan &t){
auto x = make_duration(5,false);
auto y = duration_term<T, Eval_duration3<T>, make_duration(15,false).first>(trace, t);
return b3_lessthan(x, y);
}(trace,t);
        };
        
#ifdef RTMLIB_ENABLE_MAP_SORT
  #include <string>
  #include <unordered_map>

  // Create an unordered_map of sorts (that map to integers)
  std::unordered_map<std::string, int> _mapsorttostring = {
  {"a",PROP_a},
  };

  // Create an unordered_map of sorts (that map to strings)
  std::unordered_map<int, std::string> _mapsorttoint = {
  {PROP_a,"a"},
  };
#endif

  #endif //RTM_COMPUTE_3DA2_H_

