// Mini-biblioteca de testes so assert simples, sem framework.
#pragma once
#include <cmath>
#include <cstdio>

static int g_falhas = 0;

#define CHECK(cond) \
    do { if (!(cond)) { std::printf("FALHOU %s:%d  %s\n", __FILE__, __LINE__, #cond); ++g_falhas; } } while (0)

#define CHECK_NEAR(valor, esperado, tol) \
    do { double v_ = (valor), e_ = (esperado); \
         if (std::fabs(v_ - e_) > (tol)) { \
             std::printf("FALHOU %s:%d  %s = %g (esperado %g)\n", __FILE__, __LINE__, #valor, v_, e_); ++g_falhas; } \
    } while (0)

// No fim do main devolve 0 se tudo passou.
inline int resultado()
{
    if (g_falhas == 0) std::puts("OK");
    return g_falhas == 0 ? 0 : 1;
}
