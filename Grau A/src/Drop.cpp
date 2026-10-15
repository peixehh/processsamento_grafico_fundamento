#include "Drop.h"

#include <algorithm>

void Drop::update(float dt, const World& world, float gravity)
{
    life -= dt;
    pickupDelay -= dt;
    if (body.onGround) body.vel.x -= body.vel.x * std::min(1.0f, 8.0f * dt);
    body.vel.y = std::min(body.vel.y + gravity * dt, 900.0f);
    moveAndCollide(world, body, dt);
    sprite.update(dt);
}
