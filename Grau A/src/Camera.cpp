#include "Camera.h"

#include <algorithm>

void Camera::follow(glm::vec2 target, float dt, bool snap)
{
    glm::vec2 alvo = target - glm::vec2(viewW, viewH) * 0.5f;
    pos = snap ? alvo : pos + (alvo - pos) * std::min(1.0f, 8.0f * dt);
}

void Camera::clamp(float worldW, float worldH)
{
    pos.x = std::max(0.0f, std::min(pos.x, worldW - viewW));
    pos.y = std::max(0.0f, std::min(pos.y, worldH - viewH));
}

glm::vec2 Camera::screenToWorld(double mouseX, double mouseY, int windowW, int windowH) const
{
    return pos + glm::vec2(static_cast<float>(mouseX) * viewW / windowW, static_cast<float>(mouseY) * viewH / windowH);
}
