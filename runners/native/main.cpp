/**
 * Generic main for rmtld3synth-generated monitors.
 *
 * To adapt to a different generated monitor, change the three lines
 * in the "Configuration" section below:
 *   1. Include the generated Rtm_monitor_<id>.h
 *   2. Include the generated Rtm_instrument_<id>.h
 *   3. Set WRITER_TYPE to the Writer_rtm__<id> alias from the instrument header
 *   4. Set INSTRUMENT_CLASS to the RTM_INSTRUMENT_<ID> class name
 *
 * The macros RTML_BUFFER0_SETUP() and RTML_BUFFER0_TRIGGER_PERIODIC_MONITORS()
 * are always named the same regardless of the formula synthesised.
 * For multiple monitors in one run, additional RTML_BUFFER1_*, RTML_BUFFER2_*, etc.
 * macros will be present in the respective headers.
 */

// --- Configuration: change these per generated monitor ---
#include "Rtm_monitor_3da2.h"
#include "Rtm_instrumen
t_3da2.h"
#define WRITER_TYPE      Writer_rtm__3da2
#define INSTRUMENT_CLASS RTM_INSTRUMENT_3DA2
// ---------------------------------------------------------

#include <cstdio>
#include <unistd.h>

// Global buffer declaration (satisfies the extern in the instrument header).
// Must be at file scope.
RTML_BUFFER0_SETUP()

int main() {
    // Writer used to push events into the shared buffer.
    WRITER_TYPE writer;

    // --- Push events ---
    // Use writer.push(proposition) for an event timestamped by the writer internally,
    // or writer.push(proposition, timestamp) to supply an explicit timestamp.
    //
    // Propositions are members of INSTRUMENT_CLASS::prop_t (see Rtm_instrument_<id>.h).
    // Example for this monitor (formula: duration of a in 0..15 >= 5):
    //
    //   Push 'a' active from t=0 to t=10 (in the monitor's time unit):
    writer.push(INSTRUMENT_CLASS::a, 0);
    writer.push(INSTRUMENT_CLASS::a, 9);

    // --- Set up the periodic monitor ---
    // Expanded manually to pass tbegin = 0, so the monitor evaluates from
    // the same origin as the pushed event timestamps.
    RMTLD3_reader<RTML_reader<RTML_buffer<RTM_MONITOR_3DA2_BUFFER_TYPE, RTM_MONITOR_3DA2_BUFFER_SIZE>>>
        __trace_rtm_monitor_3da2_0 = RMTLD3_reader<
            RTML_reader<RTML_buffer<RTM_MONITOR_3DA2_BUFFER_TYPE, RTM_MONITOR_3DA2_BUFFER_SIZE>>>(__buffer_rtm_monitor_3da2);
    timespan tbegin = 0;
    Rtm_monitor_3da2_0<RMTLD3_reader<RTML_reader<RTML_buffer<RTM_MONITOR_3DA2_BUFFER_TYPE, RTM_MONITOR_3DA2_BUFFER_SIZE>>>>
        rtm_mon0(200000, __trace_rtm_monitor_3da2_0, tbegin);

    // Start the monitoring thread (period defined in the generated header, here 200000 us).
    rtm_mon0.enable();

    // Wait for at least one full monitoring period before reading the verdict.
    usleep(400000); // 400 ms > 200 ms period

    // Stop the monitoring thread.
    rtm_mon0.disable();
    rtm_mon0.join();

    // --- Read verdict ---
    three_valued_type verdict = rtm_mon0.getVeredict();
    switch (verdict) {
        case T_TRUE:    printf("Verdict: TRUE  (formula satisfied)\n"); break;
        case T_FALSE:   printf("Verdict: FALSE (formula violated)\n");  break;
        case T_UNKNOWN: printf("Verdict: UNKNOWN\n");                   break;
        default:        printf("Verdict: %d (unexpected)\n", (int)verdict); break;
    }

    return 0;
}
