#pragma once
#include <glm/glm.hpp>

// Camera 2D ortografica (aula Sistemas de Coordenadas, slide de scrolling) a "janela do mundo"
// que desliza pelo mapa seguindo o personagem. Mover a camera e so mudar a matriz de projecao.
class Camera
{
public:
    glm::vec2 pos{0.0f};                    // canto superior esquerdo da janela, em unidades de mundo
    float viewW = 960.0f, viewH = 540.0f;   // tamanho da janela (resolucao logica)

    // Aproxima a janela do alvo (o centro da janela fica em 'target'). snap = true pula direto.
    void follow(glm::vec2 target, float dt, bool snap);
    // Nao deixa a janela sair do mapa (worldW x worldH, em unidades de mundo).
    void clamp(float worldW, float worldH);
    // Posicao do mouse na janela do sistema -> coordenadas de mundo.
    glm::vec2 screenToWorld(double mouseX, double mouseY, int windowW, int windowH) const;

    float left() const { return pos.x; }
    float right() const { return pos.x + viewW; }
    float top() const { return pos.y; }
    float bottom() const { return pos.y + viewH; }
};
