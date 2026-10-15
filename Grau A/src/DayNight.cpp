#include "DayNight.h"

#include <algorithm>
#include <cmath>

namespace
{
const float kTwoPi = 6.2831853f;
}

float DayNight::factor(float clock) const
{
    float p = std::fmod(clock / length + start, 1.0f);
    return std::max(0.0f, std::min(1.0f, 0.5f + 0.8f * std::sin(kTwoPi * p)));
}

// Iluminacao depende da hora do dia e da profundidade (cavernas ficam escuras).
glm::vec4 DayNight::lightAt(float clock, const World& world, int tx, int ty) const
{
    float d = factor(clock);
    float ambient = 0.40f + 0.60f * d;
    float depth = std::max(0.0f, std::min(1.0f, (ty - world.surface(tx)) / 16.0f));
    float l = ambient + (0.40f - ambient) * depth;
    float eff = d + (1.0f - d) * depth;
    glm::vec3 night(0.72f, 0.80f, 1.15f);
    glm::vec3 color = l * (night + (glm::vec3(1.0f) - night) * eff);
    return { color, 1.0f };
}
