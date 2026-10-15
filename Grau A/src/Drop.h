#pragma once
#include <string>

#include <glm/glm.hpp>

#include "Body.h"
#include "Sprite.h"
#include "World.h"

// Itens que caem dos inimigos moeda e coracao (coletados por colisao AABB com o jogador)
struct Drop
{
    enum class Kind { Coin = 0, Heart = 1 };   // tambem e a linha do spritesheet drops.png
    Kind kind = Kind::Coin;
    Body body;
    Sprite sprite;
    float life = 30.0f;
    float pickupDelay = 0.4f;

    // Passa o tempo atrito no chao, gravidade, colisao com os blocos e animacao.
    void update(float dt, const World& world, float gravity);
};

// Numero que sobe e some (dano causado/recebido, cura)
struct FloatText
{
    std::string text;
    glm::vec2 pos{0.0f};
    float life = 0.9f, maxLife = 0.9f;
    glm::vec4 color{1.0f};
};
