#include "Player.h"

#include <algorithm>
#include <cmath>

PlayerEvents Player::update(float dt, const Input& in, const World& world, const PlayerParams& p)
{
    PlayerEvents ev;

    float dir = 0.0f;
    if (invuln < 1.0f)   // logo apos o dano o controle fica travado (recuo)
    {
        dir = in.moveX;
        float k = body.onGround ? 18.0f : 8.0f;
        body.vel.x += (dir * p.speed - body.vel.x) * std::min(1.0f, k * dt);
        if (in.jump && body.onGround) body.vel.y = -p.jump;
    }
    if (dir != 0.0f) facing = dir > 0 ? 1 : -1;

    body.vel.y = std::min(body.vel.y + p.gravity * dt, 900.0f);
    moveAndCollide(world, body, dt);

    // Dano de queda guarda o ponto mais alto desde que saiu do chao e, ao pousar,
    // compara com a altura do pouso. Acima do limite seguro perde coracoes.
    if (!body.onGround)
    {
        if (wasGround) apexY = body.pos.y;
        else apexY = std::min(apexY, body.pos.y);
    }
    else if (!wasGround)
    {
        float fall = body.pos.y - apexY;
        if (fall > p.fallSafe)
        {
            int dmg = 1 + static_cast<int>((fall - p.fallSafe) / p.fallPerHeart);
            takeDamage(dmg, 0, false);
            ev.fallDamage = dmg;
        }
    }
    wasGround = body.onGround;

    invuln = std::max(0.0f, invuln - dt);

    // regeneracao lenta 1 coracao a cada 'regenTime' segundos sem levar dano
    if (hp < maxHp && invuln <= 0.0f)
    {
        regen += dt;
        if (regen >= p.regenTime) { hp++; regen = 0.0f; }
    }
    return ev;
}

bool Player::takeDamage(int amount, int fromDir, bool knockback)
{
    if (knockback && invuln > 0.0f) return false;   // golpes de inimigo respeitam a invulnerabilidade
    hp -= amount;
    regen = 0.0f;
    invuln = std::max(invuln, knockback ? 1.2f : 0.6f);
    if (knockback) body.vel = { fromDir * 300.0f, -330.0f };
    if (hp <= 0) hp = 0;
    return true;
}

void Player::animate(float dt)
{
    if (mining) sprite.setAnim(3);
    else if (!body.onGround) { sprite.setAnim(2); sprite.iFrame = body.vel.y < 0.0f ? 0 : 1; }
    else if (std::fabs(body.vel.x) > 25.0f) sprite.setAnim(1);
    else sprite.setAnim(0);
    sprite.flipX = facing < 0;
    sprite.update(dt);
}

bool Player::startSwing(int dir)
{
    if (swinging || swingCooldown > 0.0f) return false;
    swinging = true;
    swingT = 0.0f;
    swingFacing = dir;
    swingHits.clear();
    return true;
}

void Player::updateSword(float dt)
{
    swingCooldown = std::max(0.0f, swingCooldown - dt);
    if (!swinging) return;

    swingT += dt;
    facing = swingFacing;
    if (swingT >= kSwingDuration)
    {
        swinging = false;
        swingCooldown = 0.12f;
    }
}

bool Player::swordHitboxActive() const
{
    if (!swinging) return false;
    float p = swingT / kSwingDuration;
    return p >= 0.2f && p <= 0.9f;
}

// Durante o balanco existe uma hitbox AABB na frente do jogador (o Game testa contra cada inimigo).
AABB Player::swordHitbox() const
{
    glm::vec2 c = center();
    const float range = 76.0f;
    if (swingFacing > 0) return { {c.x, c.y - 56.0f}, {c.x + range, c.y + 30.0f} };
    return { {c.x - range, c.y - 56.0f}, {c.x, c.y + 30.0f} };
}

bool Player::alreadyHit(int enemyId) const
{
    return std::find(swingHits.begin(), swingHits.end(), enemyId) != swingHits.end();
}
