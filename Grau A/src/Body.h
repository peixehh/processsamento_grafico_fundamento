#pragma once
#include <glm/glm.hpp>

#include "AABB.h"
#include "World.h"

// Corpo fisico ponto (pos) = centro da base dos pes. A hitbox e menor que o
// quad do sprite (o desenho tem margem transparente), igual recomendado no enunciado.
struct Body
{
    glm::vec2 pos{0.0f};
    glm::vec2 vel{0.0f};
    float hw = 8.0f;       // meia largura da hitbox
    float h = 44.0f;       // altura da hitbox
    bool onGround = false;

    AABB box() const { return { {pos.x - hw, pos.y - h}, {pos.x + hw, pos.y} }; }
};

// Move o corpo eixo por eixo e resolve as colisoes AABB com os blocos solidos.
void moveAndCollide(const World& world, Body& b, float dt);
