#include "check.h"
#include "arena.h"
#include "Enemy.h"

#include <random>

int main()
{
    const float dt = 1.0f / 60.0f;
    World w;
    montarArena(w);

    EnemyConfig slimeCfg;
    slimeCfg.hp = 4;
    slimeCfg.damage = 1;
    slimeCfg.nFrames = 4;
    slimeCfg.nAnimations = 1;
    slimeCfg.frames = { 4 };
    slimeCfg.fps = { 0 };
    EnemyConfig zombieCfg;
    zombieCfg.hp = 6;
    zombieCfg.damage = 2;
    zombieCfg.nFrames = 4;
    zombieCfg.nAnimations = 1;
    zombieCfg.frames = { 4 };
    zombieCfg.fps = { 6 };

    // ---- fabricas valores do config e quantos sorteios cada uma consome ----
    {
        std::mt19937 a(12345), b(12345);
        Enemy z = Enemy::makeZombie({ 100.0f, 640.0f }, -1, 9, nullptr, zombieCfg, a);
        std::uniform_real_distribution<float> u(0.0f, 1.0f);
        float hop = 2.0f + u(b) * 3.0f;       // o sorteio define quando troca de direcao
        CHECK(z.type == Enemy::Type::Zombie && z.id == 9 && z.dir == -1);
        CHECK(z.hp == 6 && z.maxHp == 6 && z.damage == 2);
        CHECK_NEAR(z.body.hw, 11, 1e-6);
        CHECK_NEAR(z.body.h, 62, 1e-6);
        CHECK_NEAR(z.sprite.size.x, 72, 1e-6);
        CHECK(z.hopTimer == hop);
        CHECK(a() == b());                    // consumiu exatamente 1 sorteio

        std::mt19937 c(777), d(777);
        Enemy s = Enemy::makeSlime({ 100.0f, 640.0f }, 1, 3, nullptr, slimeCfg, c);
        float hopS = 0.5f + u(d);
        CHECK(s.type == Enemy::Type::Slime && s.id == 3 && s.dir == 1);
        CHECK(s.hp == 4 && s.maxHp == 4 && s.damage == 1);
        CHECK_NEAR(s.body.hw, 13, 1e-6);
        CHECK_NEAR(s.body.h, 20, 1e-6);
        CHECK_NEAR(s.sprite.size.x, 32, 1e-6);
        CHECK(s.hopTimer == hopS);
        CHECK(c() == d());                    // consumiu exatamente 1 sorteio
    }

    // ---- sumir longe demais, zumbi tambem some de dia quando esta a mais de 600 px ----
    {
        std::mt19937 rng(1);
        Enemy s = Enemy::makeSlime({ 1000.0f, 640.0f }, 1, 1, nullptr, slimeCfg, rng);
        Enemy z = Enemy::makeZombie({ 1000.0f, 640.0f }, 1, 2, nullptr, zombieCfg, rng);
        CHECK(!s.shouldDespawn(1000.0f + 1999.0f, 0.9f));
        CHECK(s.shouldDespawn(1000.0f + 2001.0f, 0.9f));
        CHECK(!s.shouldDespawn(1000.0f + 700.0f, 0.9f));    // slime nao liga para o dia
        CHECK(z.shouldDespawn(1000.0f + 700.0f, 0.9f));     // zumbi de dia, longe
        CHECK(!z.shouldDespawn(1000.0f + 500.0f, 0.9f));    // de dia, mas perto
        CHECK(!z.shouldDespawn(1000.0f + 700.0f, 0.2f));    // de noite fica
        CHECK(z.shouldDespawn(1000.0f - 2001.0f, 0.2f));    // longe demais para o outro lado
    }

    // ---- recuo/pisca diminuem com o tempo ----
    {
        std::mt19937 rng(1);
        Enemy s = Enemy::makeSlime({ 200.0f, 640.0f }, 1, 1, nullptr, slimeCfg, rng);
        EnemyEnv env{ w, rng, { 416.0f, 640.0f }, 1500.0f, 75.0f, 0.0f };
        s.stun = 0.3f;
        s.flash = 0.15f;
        s.update(0.1f, env);
        CHECK_NEAR(s.stun, 0.2, 1e-5);
        CHECK_NEAR(s.flash, 0.05, 1e-5);
    }

    // ---- slime pula em direcao ao jogador ----
    {
        std::mt19937 rng(1);
        Enemy s = Enemy::makeSlime({ 6 * 32.0f, 640.0f }, -1, 1, nullptr, slimeCfg, rng);
        EnemyEnv env{ w, rng, { 13 * 32.0f, 640.0f }, 1500.0f, 75.0f, 0.0f };
        float x0 = s.body.pos.x;
        bool pulou = false;
        for (int i = 0; i < 150; ++i)              // 2,5 s o primeiro pulo sai em ate 1,5 s e ainda nao passa do jogador
        {
            s.update(dt, env);
            if (s.body.vel.y < -300.0f) pulou = true;
        }
        CHECK(pulou);
        CHECK(s.dir == 1);                         // virou para o jogador (que esta a 224 px, < 650)
        CHECK(s.body.pos.x > x0 + 20.0f);
        CHECK(s.sprite.flipX == false);
    }

    // ---- zumbi persegue o jogador e pula o degrau de 1 bloco (coluna 15) ----
    {
        std::mt19937 rng(1);
        Enemy z = Enemy::makeZombie({ 13 * 32.0f, 640.0f }, 1, 1, nullptr, zombieCfg, rng);
        EnemyEnv env{ w, rng, { 28 * 32.0f, 640.0f }, 1500.0f, 75.0f, 0.0f };   // jogador a 480 px (< 800) persegue
        bool pulou = false;
        for (int i = 0; i < 240; ++i)              // 4 s ele anda no maximo ~300 px e nao alcanca o jogador
        {
            z.update(dt, env);
            if (z.body.vel.y < -300.0f) pulou = true;
        }
        CHECK(pulou);
        CHECK(z.body.pos.x > 15 * 32.0f + 16.0f);  // passou por cima do degrau
        CHECK(z.dir == 1);
    }
    return resultado();
}
