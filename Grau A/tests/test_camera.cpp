#include "check.h"
#include "Camera.h"

int main()
{
    Camera cam;   // janela do mundo de 960 x 540
    CHECK_NEAR(cam.viewW, 960, 1e-6);
    CHECK_NEAR(cam.viewH, 540, 1e-6);

    // snap a janela centraliza o alvo
    cam.follow({ 4800.0f, 1000.0f }, 0.016f, true);
    cam.clamp(9600.0f, 3200.0f);
    CHECK_NEAR(cam.pos.x, 4800.0 - 480.0, 1e-4);
    CHECK_NEAR(cam.pos.y, 1000.0 - 270.0, 1e-4);

    // limites do mapa esquerda/topo e direita/fundo (largura - janela)
    cam.follow({ 100.0f, 100.0f }, 0.016f, true);
    cam.clamp(9600.0f, 3200.0f);
    CHECK_NEAR(cam.pos.x, 0.0, 1e-6);
    CHECK_NEAR(cam.pos.y, 0.0, 1e-6);
    cam.follow({ 9500.0f, 3100.0f }, 0.016f, true);
    cam.clamp(9600.0f, 3200.0f);
    CHECK_NEAR(cam.pos.x, 9600.0 - 960.0, 1e-4);
    CHECK_NEAR(cam.pos.y, 3200.0 - 540.0, 1e-4);

    // acompanhamento suave anda 8*dt (no maximo 1) da distancia por quadro
    cam.pos = { 0.0f, 0.0f };
    cam.follow({ 960.0f, 540.0f }, 0.05f, false);      // alvo da janela = (480, 270), k = 0.4
    CHECK_NEAR(cam.pos.x, 192.0, 1e-4);
    CHECK_NEAR(cam.pos.y, 108.0, 1e-4);
    cam.pos = { 0.0f, 0.0f };
    cam.follow({ 960.0f, 540.0f }, 1.0f, false);       // k limitado a 1 chega direto
    CHECK_NEAR(cam.pos.x, 480.0, 1e-4);
    CHECK_NEAR(cam.pos.y, 270.0, 1e-4);

    // mundo menor que a janela os limites ficam invertidos e a camera fica em 0 (nao fica negativa)
    cam.follow({ 250.0f, 150.0f }, 0.016f, true);
    cam.clamp(500.0f, 300.0f);
    CHECK_NEAR(cam.pos.x, 0.0, 1e-6);
    CHECK_NEAR(cam.pos.y, 0.0, 1e-6);

    // bordas da janela
    cam.pos = { 100.0f, 50.0f };
    CHECK_NEAR(cam.left(), 100, 1e-6);
    CHECK_NEAR(cam.right(), 1060, 1e-6);
    CHECK_NEAR(cam.top(), 50, 1e-6);
    CHECK_NEAR(cam.bottom(), 590, 1e-6);

    // mouse janela do sistema de 1920 x 1080 para a resolucao logica de 960 x 540
    glm::vec2 m = cam.screenToWorld(960.0, 540.0, 1920, 1080);
    CHECK_NEAR(m.x, 100 + 480, 1e-4);
    CHECK_NEAR(m.y, 50 + 270, 1e-4);
    m = cam.screenToWorld(0.0, 0.0, 1920, 1080);
    CHECK_NEAR(m.x, 100, 1e-4);
    CHECK_NEAR(m.y, 50, 1e-4);
    m = cam.screenToWorld(480.0, 270.0, 960, 540);     // janela do mesmo tamanho 1 pixel = 1 unidade
    CHECK_NEAR(m.x, 580, 1e-4);
    CHECK_NEAR(m.y, 320, 1e-4);
    return resultado();
}
