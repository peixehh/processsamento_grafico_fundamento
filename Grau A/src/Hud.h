#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Drop.h"
#include "Inventory.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "Texturas.h"

// Dados que o HUD mostra (o Game preenche a cada quadro).
struct HudInfo
{
    const Inventory* inventory = nullptr;
    int hp = 0, maxHp = 0;
    int coins = 0, kills = 0, blocksMined = 0;
    int depth = 0;           // blocos abaixo da superficie
    bool isDay = true;
    float survived = 0.0f;   // segundos de partida
};

// Telas de entrada e de game over, barra de itens, coracoes e contadores.
// Tudo usa a projecao de TELA (origem no canto superior esquerdo, y para baixo).
class Hud
{
public:
    void init(SpriteRenderer* renderer, TextRenderer* text, const Texturas* tex,
              int sizeSmall, int sizeMedium, int sizeBig, float viewW, float viewH);

    void beginScreen();                          // projecao de tela para o texto (o Game ajusta a dos sprites)
    void drawTitle(float clock);
    void drawPlaying(const HudInfo& info);       // barra de itens, coracoes e contadores
    void drawGameOver(const HudInfo& info, float clock);
    // numeros de dano/cura ficam em coordenadas de MUNDO, entao usam a projecao da camera
    void drawFloatTexts(const std::vector<FloatText>& texts, const Camera& camera);

private:
    void drawText(const std::string& s, float x, float y, int size, glm::vec4 color = {1, 1, 1, 1});
    void drawTextCentered(const std::string& s, float y, int size, glm::vec4 color = {1, 1, 1, 1});
    void drawTextRight(const std::string& s, float rightX, float y, int size, glm::vec4 color = {1, 1, 1, 1});

    SpriteRenderer* renderer_ = nullptr;
    TextRenderer* text_ = nullptr;
    const Texturas* tex_ = nullptr;
    int sizeSmall_ = 16, sizeMedium_ = 32, sizeBig_ = 80;
    float viewW_ = 960.0f, viewH_ = 540.0f;
};
