/**
 * mon_template.cpp - Translation unit GENÉRICA para um monitor RMTLD3.
 *
 * Os cabeçalhos gerados pelo rmtld3synth têm nomes baseados num hash da
 * fórmula (ex.: 5438) que MUDA sempre que a fórmula é regenerada. Para não
 * andar a editar nomes à mão, este ficheiro é compilado com o hash e os
 * caminhos descobertos pelo build.sh e passados em macros. Regenerar a
 * fórmula só exige recompilar.
 *
 * Macros esperadas (definidas pelo build.sh):
 *   MON_COMPUTE_HDR    "<caminho>/Rtm_compute_<hash>.h"
 *   MON_INSTRUMENT_HDR "<caminho>/Rtm_instrument_<hash>.h"
 *   MON_MONITOR_HDR    "<caminho>/Rtm_monitor_<hash>.h"
 *   MON_H              hash em minúsculas (ex.: 5438)
 *   MON_HU             hash em MAIÚSCULAS (ex.: 5438)
 *   MON_PREFIX         prefixo das funções expostas (ex.: mon)
 *   HAS_PROP_B         1 se a fórmula tem a proposição 'b', 0 caso contrário
 *
 * Convenção: 'a' = RPM fora do alvo; 'b' (se existir) = RPM no alvo.
 */
#include "monitor_api.h"

/* Headers core da rtmlib (precisa -I rtmlib/src). */
#include <rmtld3/rmtld3.h>
#include <event.h>
#include <circularbuffer.h>
#include <writer.h>
#include <reader.h>

#include MON_COMPUTE_HDR
#include MON_INSTRUMENT_HDR
#include MON_MONITOR_HDR

/* -- Token-pasting: reconstrói os identificadores com hash a partir de MON_H/HU -- */
#define _CAT2(a, b) a##b
#define CAT2(a, b) _CAT2(a, b)
#define _CAT3(a, b, c) a##b##c
#define CAT3(a, b, c) _CAT3(a, b, c)

#define MON_COMPUTE_FN  CAT3(_rtm_compute_, MON_H, _0)
#define MON_BUFFER      CAT2(__buffer_rtm_monitor_, MON_H)
#define MON_WRITER      CAT2(Writer_rtm__, MON_H)
#define MON_INSTRUMENT  CAT2(RTM_INSTRUMENT_, MON_HU)
#define MON_BUF_TYPE    CAT3(RTM_MONITOR_, MON_HU, _BUFFER_TYPE)
#define MON_BUF_SIZE    CAT3(RTM_MONITOR_, MON_HU, _BUFFER_SIZE)
#define MON_PUSH_FN     CAT2(MON_PREFIX, _push)
#define MON_EVAL_FN     CAT2(MON_PREFIX, _eval)

/* Define a instância global do buffer deste monitor. */
RTML_BUFFER0_SETUP();

extern "C" void MON_PUSH_FN(int out_of_target, double t)
{
  static MON_WRITER writer;

#if HAS_PROP_B
  int sym = out_of_target ? (int)MON_INSTRUMENT::a   /* fora do alvo */
                          : (int)MON_INSTRUMENT::b;  /* dentro do alvo */
#else
  int sym = out_of_target ? (int)MON_INSTRUMENT::a : 0;
#endif

  writer.push(sym, (timespan)t);
}

extern "C" int MON_EVAL_FN(double t)
{
  typedef RMTLD3_reader<
      RTML_reader<RTML_buffer<MON_BUF_TYPE, MON_BUF_SIZE>>>
      trace_t;

  trace_t trace(MON_BUFFER);
  trace.synchronize();

  timespan ts = (timespan)t;
  three_valued_type out = T_UNKNOWN;
  if (trace.set(ts) == 0)
    out = MON_COMPUTE_FN(trace, ts);

  return (out == T_TRUE) ? MON_TRUE : (out == T_FALSE) ? MON_FALSE
                                                       : MON_UNKNOWN;
}
