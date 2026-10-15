// Roteiro de entradas e rastro de execucao. Usado pela COPIA do codigo original
// (gerada por make_orig_replay.py) e pela versao refatorada (tests/replay_game.cpp)
// as duas executam o mesmo roteiro e gravam o mesmo rastro, que tem que ser identico.
#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "World.h"

constexpr int kReplayFrames = 4800;   // 80 s de jogo a 60 quadros por segundo

// Um trecho do roteiro do quadro "de" ate "ate" (exclusivo) as entradas abaixo ficam fixas.
struct PassoInput
{
    int de, ate;
    float moveX;           // -1 esquerda, 0 parado, +1 direita
    bool pula;             // segura o pulo
    bool esq, dir;         // botoes do mouse
    float offX, offY;      // mouse em relacao ao centro do jogador
    int slot;              // item da barra (0..7)
    bool mira;             // true o mouse aponta para o inimigo mais proximo
};

inline PassoInput replayPasso(int f)
{
    static const PassoInput passos[] = {
        //    de   ate  moveX  pula   esq    dir    offX offY slot mira
        {    0,  240,   0, false, false, false,     0,   0, 0, false },   // cai de 12 blocos (dano de queda)
        {  240,  420,   1, false, false, false,     0,   0, 0, false },   // anda para a direita
        {  420,  426,   1, true,  false, false,     0,   0, 0, false },   // pula andando
        {  426,  540,   1, false, false, false,     0,   0, 0, false },
        {  540,  600,   0, false, true,  false,    36,  40, 0, false },   // minera blocos ao redor (picareta)
        {  600,  660,   0, false, true,  false,    36,  72, 0, false },
        {  660,  720,   0, false, true,  false,    68,  40, 0, false },
        {  720,  780,   0, false, true,  false,    68,  72, 0, false },
        {  780,  840,   0, false, true,  false,   -36,  40, 0, false },
        {  840,  900,   0, false, true,  false,   -36,  72, 0, false },
        {  900,  960,   0, false, false, false,     0,   0, 2, false },   // escolhe terra
        {  960, 1020,   0, false, false, true,     36,  40, 2, false },   // coloca blocos de volta
        { 1020, 1080,   0, false, false, true,     36,  72, 2, false },
        { 1080, 1140,   0, false, false, true,     68,  40, 2, false },
        { 1140, 1200,   0, false, false, true,     68,  72, 2, false },
        { 1200, 1500,  -1, false, false, false,     0,   0, 0, false },   // volta para a esquerda
        { 1500, 1506,  -1, true,  false, false,     0,   0, 0, false },
        { 1506, 1800,   0, false, false, false,     0,   0, 0, false },   // espera
        { 1800, 2700,   1, true,  false, false,     0,   0, 0, false },   // anda pulando (pousos, degraus, arvores)
        { 2700, 3000,   0, false, false, false,     0,   0, 1, false },   // espada na mao, parado (anoitece)
        { 3000, 3600,   0, false, true,  false,     0,   0, 1, true  },   // golpeia o inimigo mais proximo
        { 3600, 3700,  -1, false, false, false,     0,   0, 1, false },   // recua
        { 3700, 4800,   0, false, true,  false,     0,   0, 1, true  },   // volta a golpear
    };
    for (const PassoInput& p : passos)
        if (f >= p.de && f < p.ate) return p;
    return PassoInput{ f, f + 1, 0, false, false, false, 0, 0, 0, false };
}

struct ReplayEstado
{
    bool ativo = false;
    PassoInput cur{};
    glm::vec2 mouse{0.0f};   // mouse em coordenadas de MUNDO, calculado no INICIO de cada quadro (igual nas duas versoes)

    // Responde "a tecla k esta apertada?" para o codigo original, que le o teclado direto.
    bool tecla(int k) const
    {
        switch (k)
        {
        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:  return cur.moveX < 0.0f;
        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT: return cur.moveX > 0.0f;
        case GLFW_KEY_W:
        case GLFW_KEY_UP:
        case GLFW_KEY_SPACE: return cur.pula;
        default:             return false;
        }
    }
};
inline ReplayEstado g_replay;

inline uint64_t replayHashMundo(const World& w)
{
    uint64_t h = 1469598103934665603ull;   // FNV-1a
    for (int y = 0; y < w.H; ++y)
        for (int x = 0; x < w.W; ++x)
        {
            h ^= w.get(x, y);
            h *= 1099511628211ull;
        }
    return h;
}

// Grava o quadro atual da janela em <prefixo>_<nome>.ppm (virado o OpenGL le de baixo para cima).
inline void replayImagem(GLFWwindow* janela, const char* prefixo, const char* nome)
{
    int largura = 0, altura = 0;
    glfwGetFramebufferSize(janela, &largura, &altura);
    std::vector<unsigned char> px(static_cast<size_t>(largura) * altura * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, largura, altura, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    std::string caminho = std::string(prefixo) + "_" + nome + ".ppm";
    FILE* f = std::fopen(caminho.c_str(), "wb");
    if (!f) return;
    std::fprintf(f, "P6\n%d %d\n255\n", largura, altura);
    for (int y = altura - 1; y >= 0; --y)
        std::fwrite(&px[static_cast<size_t>(y) * largura * 3], 1, static_cast<size_t>(largura) * 3, f);
    std::fclose(f);
}
