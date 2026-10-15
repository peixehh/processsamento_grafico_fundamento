#include "check.h"
#include "golden.h"
#include "AABB.h"

int main()
{
    // mesmos casos do codigo original (golden) a caixa (0,0)-(10,10) contra cinco outras
    AABB a{ {0, 0}, {10, 10} };
    const AABB outros[5] = { { {5, 5}, {15, 15} }, { {10, 0}, {20, 10} }, { {20, 20}, {30, 30} },
                             { {2, 2}, {4, 4} }, { {-5, -5}, {0, 0} } };
    for (int i = 0; i < 5; ++i)
    {
        CHECK(checkCollision(a, outros[i]) == kGoldenAABB[i]);
        CHECK(checkCollision(outros[i], a) == kGoldenAABB[i]);   // simetrica
    }
    CHECK(!checkCollision(a, outros[1]));   // encostar na borda NAO e colisao (o slide usa >=, o jogo usa estrito)

    const glm::vec2 pontos[4] = { {5, 5}, {10, 10}, {10.01f, 5}, {-1, 5} };
    for (int i = 0; i < 4; ++i) CHECK(a.contains(pontos[i]) == kGoldenContem[i]);

    // fromCenter pMin = centro - tamanho/2 e pMax = centro + tamanho/2 (slide ColisoesAABB)
    AABB c = AABB::fromCenter({ 10, 20 }, { 4, 6 });
    CHECK_NEAR(c.pMin.x, 8, 1e-6);
    CHECK_NEAR(c.pMin.y, 17, 1e-6);
    CHECK_NEAR(c.pMax.x, 12, 1e-6);
    CHECK_NEAR(c.pMax.y, 23, 1e-6);
    return resultado();
}
