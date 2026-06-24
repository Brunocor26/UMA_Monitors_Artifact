/**
 * monitor_api.h - API mínima (C ABI) sobre o monitor RMTLD3.
 *
 * O monitor é compilado com mon_template.cpp que inclui os
 * cabeçalhos gerados pelo rmtld3synth (cujos nomes têm um hash que muda a
 * cada regeneração). O servidor usa-o só através destas funções.
 *
 * Fórmula monitorizada:  a -> (a U[<10s] b)
 *   'a' = RPM fora do alvo;  'b' = RPM no alvo.
 *   Veredito FALSE  -> saiu do alvo e NÃO recuperou em <10s (violação).
 *   Veredito TRUE   -> dentro do alvo, ou recuperou a tempo.
 *
 * Vereditos (lógica de três valores da rtmlib):
 *   -1 = FALSE, 0 = UNKNOWN, 1 = TRUE.
 */
#ifndef MONITOR_API_H
#define MONITOR_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define MON_FALSE   (-1)
#define MON_UNKNOWN (0)
#define MON_TRUE    (1)

/* out_of_target != 0 empurra 'a' (fora do alvo); senão empurra 'b' (no alvo).
 * t em segundos (unidade de tempo do monitor). */
void mon_push(int out_of_target, double t);

/* Avalia a fórmula no instante t e devolve o veredito normalizado. */
int  mon_eval(double t);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_API_H */
