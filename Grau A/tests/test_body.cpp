#include "check.h"
#include "golden.h"
#include "arena.h"

static void confere(const ResultadoCorpo& r, const GoldenCorpo& g)
{
    CHECK_NEAR(r.x, g.x, 1e-3);
    CHECK_NEAR(r.y, g.y, 1e-3);
    CHECK_NEAR(r.vx, g.vx, 1e-3);
    CHECK_NEAR(r.vy, g.vy, 1e-3);
    CHECK(r.noChao == g.noChao);
    CHECK_NEAR(r.minY, g.minY, 1e-3);
}

int main()
{
    // 1) igual ao codigo original (golden)
    confere(cenarioQueda(), kGoldenQueda);
    confere(cenarioParede(), kGoldenParede);
    confere(cenarioTeto(), kGoldenTeto);
    confere(cenarioDegrau(), kGoldenDegrau);

    // 2) fatos fisicos, independentes do golden
    ResultadoCorpo q = cenarioQueda();
    CHECK_NEAR(q.y, 640.0, 1e-3);    // pousou no chao (linha 20 = 640 px)
    CHECK(q.noChao);
    ResultadoCorpo p = cenarioParede();
    CHECK_NEAR(p.x, 960.0 - 11.0, 1e-3);    // borda esquerda da parede (960) menos a meia largura
    CHECK_NEAR(p.vx, 0.0, 1e-6);
    ResultadoCorpo t = cenarioTeto();
    CHECK_NEAR(t.minY, 480.0 + 62.0, 1e-3);   // a cabeca (62 px) parou embaixo do teto (480)
    ResultadoCorpo d = cenarioDegrau();
    CHECK_NEAR(d.x, 15 * 32.0 - 11.0, 1e-3);   // parou na frente do degrau
    return resultado();
}
