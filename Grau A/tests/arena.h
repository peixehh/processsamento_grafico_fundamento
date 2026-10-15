// Cenarios de fisica usados pelo golden_capture (codigo ORIGINAL) e pelos testes (codigo novo).
// Mesma arena, mesmos passos os numeros tem que bater.
#pragma once
#include <algorithm>

#include "Body.h"
#include "World.h"

struct ResultadoCorpo { float x, y, vx, vy; bool noChao; float minY; };

// Arena de 40 x 30 blocos chao em y = 640 px (linha 20), parede alta em x = 960 px (coluna 30),
// teto baixo nas colunas 10 a 12 (y de 448 a 480 px) e um degrau de 1 bloco na coluna 15.
inline void montarArena(World& w)
{
    w.generate(40, 30, 7);
    for (int y = 0; y < w.H; ++y)
        for (int x = 0; x < w.W; ++x) w.set(x, y, AIR);
    for (int x = 0; x < w.W; ++x) w.set(x, 20, STONE);
    for (int y = 10; y < 20; ++y) w.set(30, y, STONE);
    for (int x = 10; x <= 12; ++x) w.set(x, 14, STONE);
    w.set(15, 19, STONE);
}

// pos = centro da base dos pes (igual ao jogo), hitbox 22 x 62 como a do jogador
inline Body novoCorpo(float x, float y)
{
    Body b;
    b.pos = { x, y };
    b.hw = 11.0f;
    b.h = 62.0f;
    return b;
}

// Um quadro de fisica igual ao do jogo gravidade com limite de queda e depois moveAndCollide.
inline void passo(const World& w, Body& b)
{
    const float dt = 1.0f / 60.0f;
    b.vel.y = std::min(b.vel.y + 1500.0f * dt, 900.0f);
    moveAndCollide(w, b, dt);
}

inline ResultadoCorpo resumo(const Body& b, float minY)
{
    return { b.pos.x, b.pos.y, b.vel.x, b.vel.y, b.onGround, minY };
}

inline ResultadoCorpo cenarioQueda()    // cai de y = 300 e pousa no chao (y = 640)
{
    World w;
    montarArena(w);
    Body b = novoCorpo(5 * 32 + 16, 300.0f);
    float minY = b.pos.y;
    for (int i = 0; i < 90; ++i) { passo(w, b); minY = std::min(minY, b.pos.y); }
    return resumo(b, minY);
}

inline ResultadoCorpo cenarioParede()   // anda para a direita ate bater na parede (x = 960)
{
    World w;
    montarArena(w);
    Body b = novoCorpo(28 * 32.0f, 640.0f);
    b.vel.x = 240.0f;
    float minY = b.pos.y;
    for (int i = 0; i < 60; ++i) { passo(w, b); minY = std::min(minY, b.pos.y); }
    return resumo(b, minY);
}

inline ResultadoCorpo cenarioTeto()     // pula debaixo do teto baixo e bate a cabeca
{
    World w;
    montarArena(w);
    Body b = novoCorpo(11 * 32 + 16, 640.0f);
    b.vel.y = -700.0f;
    float minY = b.pos.y;
    for (int i = 0; i < 60; ++i) { passo(w, b); minY = std::min(minY, b.pos.y); }
    return resumo(b, minY);
}

inline ResultadoCorpo cenarioDegrau()   // anda ate o degrau de 1 bloco e fica parado contra ele
{
    World w;
    montarArena(w);
    Body b = novoCorpo(13 * 32.0f, 640.0f);
    b.vel.x = 100.0f;
    float minY = b.pos.y;
    for (int i = 0; i < 60; ++i) { passo(w, b); minY = std::min(minY, b.pos.y); }
    return resumo(b, minY);
}
