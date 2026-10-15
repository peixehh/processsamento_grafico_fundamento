#pragma once
#include <glm/glm.hpp>

// "Foto" do teclado e do mouse em um quadro. Game::processInput preenche, Player e as regras de
// mineracao so leem (assim nao dependem do GLFW e podem ser testados sem janela).
struct Input
{
    float moveX = 0.0f;           // -1 esquerda, 0 parado, +1 direita
    bool jump = false;            // W, seta para cima ou espaco
    bool mouseLeft = false;       // botao esquerdo minerar / golpear
    bool mouseRight = false;      // botao direito colocar bloco
    glm::vec2 mouseWorld{0.0f};   // posicao do mouse em coordenadas de mundo
};
