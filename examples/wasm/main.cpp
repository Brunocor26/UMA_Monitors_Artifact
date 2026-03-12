/**
 * main.cpp — Exemplo de uso completo do monitor RMTLD3 gerado para a fórmula:
 *
 *   (a U[10s] b)   →   "a" deve ser verdade até que "b" ocorra, dentro de 10s
 *
 * Cobre:
 *   1. Lógica de três valores  (T_TRUE / T_FALSE / T_UNKNOWN)
 *   2. Escrita no buffer       (Writer_rtm__4ddc)
 *   3. Avaliação da fórmula    (_rtm_compute_4ddc_0)
 *   4. Monitor periódico       (Rtm_monitor_4ddc_0)
 *
 * Compilação (exemplo):
 *   g++ -std=c++17 -I<rtmlib_include_dir> -o test_monitor main.cpp
 */

// ──────────────────────────────────────────────────────────────────────────────
// Includes da rtmlib (ajuste os caminhos conforme o vosso projecto)
// ──────────────────────────────────────────────────────────────────────────────
#include "../rtmlib/src/rmtld3/rmtld3.h"
#include "Rtm_compute_4ddc.h"
#include "Rtm_instrument_4ddc.h"
#include "Rtm_monitor_4ddc.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>   // usleep

// ──────────────────────────────────────────────────────────────────────────────
// 1. Instância global do buffer (exigida pelo linker — declarado extern nos .h)
// ──────────────────────────────────────────────────────────────────────────────


RTML_BUFFER0_SETUP();   // expande para a definição de __buffer_rtm_monitor_4ddc

// ──────────────────────────────────────────────────────────────────────────────
// Utilitário: converte three_valued_type para string legível
// ──────────────────────────────────────────────────────────────────────────────
static const char *tv_str(three_valued_type v) {
    switch (v) {
        case T_TRUE:    return "T_TRUE";
        case T_FALSE:   return "T_FALSE";
        case T_UNKNOWN: return "T_UNKNOWN";
        default:        return "???";
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// 2. Escrita no buffer e avaliação directa da fórmula
// ──────────────────────────────────────────────────────────────────────────────
static void test_formula_direct() {
    printf("\n=== [2] Escrita no buffer + avaliação da fórmula ===\n");

    Writer_rtm__4ddc writer;

    // A (T_TRUE,  t= 0): a@0, a@3, b@7   → b em 7s  < 10s
    writer.push(RTM_INSTRUMENT_4DDC::a,  0.0);
    writer.push(RTM_INSTRUMENT_4DDC::a,  3.0);
    writer.push(RTM_INSTRUMENT_4DDC::b,  7.0);

    // B (T_FALSE, t=20): a@20, a@22, b@31 → b em 11s > 10s
    writer.push(RTM_INSTRUMENT_4DDC::a, 20.0);
    writer.push(RTM_INSTRUMENT_4DDC::a, 22.0);
    writer.push(RTM_INSTRUMENT_4DDC::b, 31.0);

    // C (T_TRUE,  t=40): a@40, b@49       → b em  9s < 10s (limite)
    writer.push(RTM_INSTRUMENT_4DDC::a, 40.0);
    writer.push(RTM_INSTRUMENT_4DDC::b, 49.0);

    // D (T_FALSE, t=60): a@60, a@65, a@71 → sem b; trace estende-se até 71 > 70
    writer.push(RTM_INSTRUMENT_4DDC::a, 60.0);
    writer.push(RTM_INSTRUMENT_4DDC::a, 65.0);
    writer.push(RTM_INSTRUMENT_4DDC::a, 71.0);

    RTML_BUFFER0_TRIGGER_PERIODIC_MONITORS();
    __trace_rtm_monitor_4ddc_0.synchronize();

    three_valued_type result;
    timespan t;

    auto eval = [&](timespan ts, const char *label, const char *expected) {
        t = ts; result = T_UNKNOWN;
        if (__trace_rtm_monitor_4ddc_0.set(t) == 0)
            result = _rtm_compute_4ddc_0(__trace_rtm_monitor_4ddc_0, t);
        printf("%-40s = %-10s (esperado: %s)\n", label, tv_str(result), expected);
    };

    eval( 0, "Caso A (a@0,a@3,b@7   | t= 0)", "T_TRUE");
    eval(20, "Caso B (a@20,a@22,b@31 | t=20)", "T_FALSE");
    eval(40, "Caso C (a@40,b@49     | t=40)", "T_TRUE");
    eval(60, "Caso D (a@60,a@65,a@71 | t=60)", "T_FALSE");
}

// ──────────────────────────────────────────────────────────────────────────────
// 4. Monitor periódico — arranca, escreve eventos, lê veredito
// ──────────────────────────────────────────────────────────────────────────────
static void test_periodic_monitor() {
    printf("\n=== [3] Monitor periódico ===\n");

    // Prepara trace + monitor com período 200ms (gerado pela tool)
    RTML_BUFFER0_TRIGGER_PERIODIC_MONITORS();

    rtm_mon0.enable();
    printf("Monitor iniciado (período=200ms). A aguardar 2 ciclos...\n");

    Writer_rtm__4ddc writer;
    writer.push(RTM_INSTRUMENT_4DDC::b, 1.0);

    usleep(500000);   // espera 500 ms → ~2 ciclos do monitor

    printf("Veredito do monitor após 500 ms: %s\n", tv_str(rtm_mon0.getVeredict()));

    rtm_mon0.disable();
    printf("Monitor terminado.\n");
}

// ──────────────────────────────────────────────────────────────────────────────
// main
// ──────────────────────────────────────────────────────────────────────────────
int main() {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Teste completo — fórmula: (a U[10s] b)              ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    //test_three_valued_logic();
    test_formula_direct();
    //test_periodic_monitor();

    printf("\n✓ Todos os testes concluídos.\n");
    return 0;
}