/* This file was automatically generated from rmtld3synth tool version
v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14.

Settings:
out_dir -> '/out'
rtm_buffer_size -> 100
rtm_event_subtype -> 'std::underlying_type<_auto_gen_prop>::type'
version -> 'v0.7 (39aa6b1bfb6a4299a0c7ada2b1b1662773cae41c)
x86_64 GNU/Linux 2025-10-12 18:14'
fm_num_until -> 1
input_exp -> (a U[10.] b)
rtm_monitor_name_prefix -> 'rtm_#_%'
rtm_monitor_time_unit -> 's'
fm_num_prop -> 2
rtm_period -> 200000
prop_map_reverse -> (2->a) (1->b) 
rtm_min_inter_arrival_time -> 1
init -> true
unique_id_counter -> 1
rtm_max_period -> 2000000
rtm_event_type -> 'Event'
input_exp_dsl -> 'a until b within 10s'
prop_map -> (b->1) (a->2) 
gen_tests -> 'false'

Formula(s):
- (a U[10.] b)

*/

  #ifndef RTM_COMPUTE_4DDC_H_
  #define RTM_COMPUTE_4DDC_H_

#include "../../rtmlib/src/rmtld3/rmtld3.h"
#include "../../rtmlib/src/rmtld3/formulas.h"
  
  #define RTM_TIME_UNIT s
  
  // Propositions
  enum _auto_gen_prop {PROP_b = 1, PROP_a = 2, }; 
  
  template <typename T> class Eval_until_less_3 {
  public:
    static three_valued_type eval_phi1(T &trace, timespan &t) {
      auto sf = prop<T>(trace, PROP_a, t);
      return sf;
    };
    static three_valued_type eval_phi2(T &trace, timespan &t) {
      auto sf = prop<T>(trace, PROP_b, t);
      return sf;
    };
  };

        template<class T>
        three_valued_type _rtm_compute_4ddc_0 (T &trace, timespan &t) {
          return until_less<T, Eval_until_less_3<T>, 10>(trace, t);
        };
        
#ifdef RTMLIB_ENABLE_MAP_SORT
  #include <string>
  #include <unordered_map>

  // Create an unordered_map of sorts (that map to integers)
  std::unordered_map<std::string, int> _mapsorttostring = {
  {"b",PROP_b},
  {"a",PROP_a},
  };

  // Create an unordered_map of sorts (that map to strings)
  std::unordered_map<int, std::string> _mapsorttoint = {
  {PROP_b,"b"},
  {PROP_a,"a"},
  };
#endif

  #endif //RTM_COMPUTE_4DDC_H_

