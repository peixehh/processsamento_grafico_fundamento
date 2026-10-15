#include "Enemy.h"

#include <algorithm>
#include <cmath>

Enemy Enemy::makeSlime(glm::vec2 pos, int dir, int id, const Texture* tex, const EnemyConfig& cfg, std::mt19937& rng)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    Enemy e;
    e.type = Type::Slime;
    e.id = id;
    e.body.pos = pos;
    e.dir = dir;
    e.body.hw = 13.0f;
    e.body.h = 20.0f;
    e.hp = e.maxHp = cfg.hp;
    e.damage = cfg.damage;
    e.sprite.setup(tex, cfg.nFrames, cfg.nAnimations, cfg.frames, cfg.fps);
    e.sprite.size = { 32.0f, 32.0f };
    e.hopTimer = 0.5f + u(rng);
    return e;
}

Enemy Enemy::makeZombie(glm::vec2 pos, int dir, int id, const Texture* tex, const EnemyConfig& cfg, std::mt19937& rng)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    Enemy e;
    e.type = Type::Zombie;
    e.id = id;
    e.body.pos = pos;
    e.dir = dir;
    e.body.hw = 11.0f;
    e.body.h = 62.0f;
    e.hp = e.maxHp = cfg.hp;
    e.damage = cfg.damage;
    e.sprite.setup(tex, cfg.nFrames, cfg.nAnimations, cfg.frames, cfg.fps);
    e.sprite.size = { 72.0f, 72.0f };
    e.hopTimer = 2.0f + u(rng) * 3.0f;
    return e;
}

bool Enemy::shouldDespawn(float playerX, float dayFactor) const
{
    float dx = playerX - body.pos.x;
    return std::fabs(dx) > 2000.0f || (type == Type::Zombie && dayFactor > 0.55f && std::fabs(dx) > 600.0f);
}

void Enemy::update(float dt, const EnemyEnv& env)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    float dx = env.playerPos.x - body.pos.x;
    bool zombie = type == Type::Zombie;

    flash = std::max(0.0f, flash - dt);
    stun = std::max(0.0f, stun - dt);

    if (!zombie)
    {
        // ---- slime pula em direcao ao jogador ----
        if (body.onGround)
        {
            body.vel.x -= body.vel.x * std::min(1.0f, 12.0f * dt);   // atrito no chao
            if (stun <= 0.0f)
            {
                hopTimer -= dt;
                if (hopTimer <= 0.0f)
                {
                    dir = std::fabs(dx) < 650.0f ? (dx >= 0.0f ? 1 : -1) : (u(env.rng) < 0.5f ? -1 : 1);
                    body.vel = { dir * 140.0f, -430.0f };
                    hopTimer = 0.9f + u(env.rng) * 0.8f;
                }
            }
        }
        else if (stun <= 0.0f)   // no ar continua empurrando para frente (passa por degraus de 1 bloco)
            body.vel.x += (dir * 140.0f - body.vel.x) * std::min(1.0f, 6.0f * dt);
    }
    else
    {
        // ---- zumbi persegue o jogador andando e pula degraus de 1 bloco ----
        bool chase = std::fabs(dx) < 800.0f;
        hopTimer -= dt;
        if (!chase && hopTimer <= 0.0f) { dir = u(env.rng) < 0.5f ? -1 : 1; hopTimer = 2.0f + u(env.rng) * 3.0f; }
        if (chase) dir = dx >= 0.0f ? 1 : -1;

        if (stun <= 0.0f)
        {
            float target = dir * (chase ? env.zombieSpeed : env.zombieSpeed * 0.4f);
            body.vel.x += (target - body.vel.x) * std::min(1.0f, 8.0f * dt);
            if (body.onGround)
            {
                int ax = static_cast<int>(std::floor((body.pos.x + dir * (body.hw + 6.0f)) / TILE));
                int fy = static_cast<int>(std::floor((body.pos.y - 4.0f) / TILE));
                if (env.world.solid(ax, fy) && !env.world.solid(ax, fy - 1) && !env.world.solid(ax, fy - 2))
                    body.vel.y = -460.0f;
            }
        }
        else body.vel.x -= body.vel.x * std::min(1.0f, 6.0f * dt);
    }

    body.vel.y = std::min(body.vel.y + env.gravity * dt, 900.0f);
    moveAndCollide(env.world, body, dt);

    // animacao
    if (!zombie)
    {
        if (body.onGround) sprite.iFrame = static_cast<int>(env.clock * 3.0f) % 2;   // respirando
        else sprite.iFrame = body.vel.y < 0.0f ? 2 : 3;                               // esticando no ar
        sprite.flipX = dir < 0;
    }
    else
    {
        if (std::fabs(body.vel.x) > 10.0f) sprite.update(dt);
        else sprite.iFrame = 0;
        sprite.flipX = dir < 0;
    }
}
