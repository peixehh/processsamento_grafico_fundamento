#pragma once
#include "SpriteRenderer.h"

// Camada de fundo com parallax (aula Camadas e Parallax). Quanto mais distante a camada,
// menor a taxa de rolagem (rate) ela se move menos que a camera e passa sensacao de profundidade.
class Layer
{
public:
    enum class Mode { Sky, Day, Night, Fixed };

    // Janela de textura de uma camada onde comeca (u0), quanto cabe na tela (uScale) e o deslocamento vertical (y)
    struct Janela { float u0, uScale, y; };

    Texture tex;
    float rate = 0.0f;    // fracao do movimento horizontal da camera (0 = parada, 1 = igual ao mundo)
    float vrate = 0.0f;   // idem na vertical (o fundo sobe quando o jogador desce)
    float drift = 0.0f;   // deslocamento proprio em px/s (nuvens)
    Mode mode = Mode::Fixed;

    // A imagem e esticada para ter a altura da tela, a largura segue a proporcao.
    // Em vez de mover o quad, move-se a janela de textura (GL_REPEAT emenda a imagem).
    Janela computeWindow(float camX, float camY, float refCamY, float time, float viewW, float viewH) const
    {
        float layerW = tex.w * (viewH / static_cast<float>(tex.h));
        Janela j;
        j.u0 = (camX * rate + time * drift) / layerW;   // deslocamento da janela de textura
        j.uScale = viewW / layerW;                       // quanto da textura cabe na tela
        j.y = -(camY - refCamY) * vrate;
        return j;
    }

    void draw(SpriteRenderer& r, float camX, float camY, float refCamY, float time,
              float viewW, float viewH, const glm::vec4& tint) const
    {
        Janela j = computeWindow(camX, camY, refCamY, time, viewW, viewH);
        r.draw(tex, {0.0f, j.y}, {viewW, viewH}, {j.u0, 0.0f}, {j.uScale, 1.0f}, tint);
    }
};
