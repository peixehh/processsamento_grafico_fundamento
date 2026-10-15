#pragma once
#include <vector>

#include "SpriteRenderer.h"

// Sprite animado a partir de um spritesheet (aula Camadas e Sprites).
// Cada LINHA da imagem e uma animacao (parado, andando, pulando...) e cada
// COLUNA e um quadro. Como nos slides dx = 1/nFrames, dy = 1/nAnimations e
// o offset de textura e (iFrame * dx, iAnimation * dy).
class Sprite
{
public:
    const Texture* tex = nullptr;
    int nFrames = 1, nAnimations = 1;   // colunas e linhas do spritesheet
    std::vector<int> frameCount;        // quantos quadros cada animacao usa
    std::vector<float> fps;             // velocidade de cada animacao (0 = controlada a mao)
    int iAnimation = 0, iFrame = 0;
    bool flipX = false;
    glm::vec2 size{32.0f};              // tamanho do quad em unidades de mundo

    void setup(const Texture* t, int frames, int animations, const std::vector<float>& framesPerAnim,
               const std::vector<float>& speeds)
    {
        tex = t; nFrames = frames; nAnimations = animations;
        frameCount.clear();
        for (float f : framesPerAnim) frameCount.push_back(static_cast<int>(f));
        fps = speeds;
        frameCount.resize(nAnimations, 1);
        fps.resize(nAnimations, 0.0f);
    }

    void setAnim(int a)
    {
        if (a == iAnimation) return;
        iAnimation = a; iFrame = 0; timer_ = 0.0f;
    }

    // iFrame = (iFrame + 1) % quadros da animacao, no ritmo do fps (slide Animacao por Sprites)
    void update(float dt)
    {
        if (fps[iAnimation] <= 0.0f) return;
        timer_ += dt;
        float spf = 1.0f / fps[iAnimation];
        while (timer_ >= spf)
        {
            timer_ -= spf;
            iFrame = (iFrame + 1) % frameCount[iAnimation];
        }
    }

    void draw(SpriteRenderer& r, glm::vec2 pos, glm::vec4 tint = {1, 1, 1, 1}) const
    {
        float dx = 1.0f / nFrames, dy = 1.0f / nAnimations;
        r.draw(*tex, pos, size, {iFrame * dx, iAnimation * dy}, {dx, dy}, tint, flipX);
    }

private:
    float timer_ = 0.0f;
};
