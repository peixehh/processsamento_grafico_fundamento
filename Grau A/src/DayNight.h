#pragma once
#include <glm/glm.hpp>

#include "World.h"

// Ciclo dia/noite e iluminacao. Sem relogio proprio quem chama passa o tempo do jogo (clock, em segundos).
class DayNight
{
public:
    float length = 120.0f;   // segundos para um dia completo
    float start = 0.12f;     // fase inicial (0.25 = meio-dia, 0.75 = meia-noite)

    float factor(float clock) const;                                            // 0 = noite, 1 = dia
    glm::vec4 lightAt(float clock, const World& world, int tx, int ty) const;   // cor de iluminacao de um bloco
};
