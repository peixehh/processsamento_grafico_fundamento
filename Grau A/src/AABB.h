#pragma once
#include <glm/glm.hpp>

// Caixa alinhada aos eixos (hitbox), como no slide de colisoes AABB
// pMin = canto minimo (xmin, ymin) e pMax = canto maximo (xmax, ymax).
struct AABB
{
    glm::vec2 pMin{0.0f};
    glm::vec2 pMax{0.0f};

    // pMin = centro - tamanho/2 e pMax = centro + tamanho/2 (como no slide)
    static AABB fromCenter(glm::vec2 center, glm::vec2 size)
    {
        return { center - size * 0.5f, center + size * 0.5f };
    }

    bool contains(glm::vec2 p) const
    {
        return p.x >= pMin.x && p.x <= pMax.x && p.y >= pMin.y && p.y <= pMax.y;
    }
};

// Ha colisao quando os intervalos se sobrepoem nos DOIS eixos (slide ColisoesAABB).
// Aqui a comparacao e estrita encostar na borda nao conta como colisao.
inline bool checkCollision(const AABB& one, const AABB& two)
{
    bool collisionX = one.pMin.x < two.pMax.x && one.pMax.x > two.pMin.x;
    bool collisionY = one.pMin.y < two.pMax.y && one.pMax.y > two.pMin.y;
    return collisionX && collisionY;
}
