#include "Body.h"

#include <cmath>

namespace
{
// Percorre os blocos que tocam a hitbox e empurra o corpo para fora do bloco
// pelo lado de onde ele veio (depende do sinal da velocidade no eixo testado).
void resolve(const World& w, Body& b, bool axisX)
{
    AABB bx = b.box();
    int x0 = static_cast<int>(std::floor(bx.pMin.x / TILE)), x1 = static_cast<int>(std::floor(bx.pMax.x / TILE));
    int y0 = static_cast<int>(std::floor(bx.pMin.y / TILE)), y1 = static_cast<int>(std::floor(bx.pMax.y / TILE));

    for (int ty = y0; ty <= y1; ++ty)
        for (int tx = x0; tx <= x1; ++tx)
        {
            if (!w.solid(tx, ty)) continue;
            AABB t{ {float(tx * TILE), float(ty * TILE)}, {float((tx + 1) * TILE), float((ty + 1) * TILE)} };
            if (!checkCollision(bx, t)) continue;

            if (axisX)
            {
                if (b.vel.x > 0.0f)      b.pos.x = t.pMin.x - b.hw;
                else if (b.vel.x < 0.0f) b.pos.x = t.pMax.x + b.hw;
                b.vel.x = 0.0f;
            }
            else
            {
                if (b.vel.y > 0.0f)      { b.pos.y = t.pMin.y; b.onGround = true; }   // pousou no chao
                else if (b.vel.y < 0.0f) { b.pos.y = t.pMax.y + b.h; }               // bateu a cabeca
                b.vel.y = 0.0f;
            }
            bx = b.box();
        }
}
}  // namespace

void moveAndCollide(const World& world, Body& b, float dt)
{
    b.onGround = false;
    b.pos.x += b.vel.x * dt;     // 1) anda no X e resolve
    resolve(world, b, true);
    b.pos.y += b.vel.y * dt;     // 2) anda no Y e resolve
    resolve(world, b, false);
}
