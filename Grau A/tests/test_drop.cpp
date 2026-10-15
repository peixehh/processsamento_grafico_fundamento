#include "check.h"
#include "arena.h"
#include "Drop.h"

#include <cmath>

int main()
{
    World w;
    montarArena(w);

    Drop d;
    d.kind = Drop::Kind::Coin;
    d.body = novoCorpo(5 * 32 + 16, 300.0f);
    d.body.hw = 9.0f;
    d.body.h = 20.0f;
    d.body.vel = { 110.0f, -300.0f };
    d.sprite.setup(nullptr, 4, 2, { 4, 4 }, { 8, 5 });
    d.sprite.setAnim(0);

    float xInicial = d.body.pos.x;
    for (int i = 0; i < 180; ++i) d.update(1.0f / 60.0f, w, 1500.0f);

    CHECK_NEAR(d.life, 30.0 - 3.0, 1e-3);          // 180 quadros = 3 s
    CHECK(d.pickupDelay < 0.0f);                   // a espera de coleta (0,4 s) ja acabou
    CHECK(d.body.onGround);                        // caiu no chao
    CHECK_NEAR(d.body.pos.y, 640.0, 1e-3);
    CHECK(std::fabs(d.body.vel.x) < 1.0f);         // o atrito no chao parou o deslize
    CHECK(d.body.pos.x > xInicial);                // andou para a direita enquanto voava
    return resultado();
}
