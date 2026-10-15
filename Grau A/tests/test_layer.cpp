#include "check.h"
#include "Layer.h"

int main()
{
    // Os numeros do enunciado do parallax u0 = (camX * taxa + tempo * deriva) / larguraDaCamada
    Layer l;
    l.tex.w = 1920;   // so os tamanhos importam aqui (nenhum OpenGL)
    l.tex.h = 540;
    l.rate = 0.4f;
    l.vrate = 0.2f;
    l.drift = 8.0f;
    Layer::Janela j = l.computeWindow(100.0f, 50.0f, 20.0f, 10.0f, 960.0f, 540.0f);
    CHECK_NEAR(j.u0, (100.0 * 0.4 + 10.0 * 8.0) / 1920.0, 1e-6);   // 0.0625
    CHECK_NEAR(j.uScale, 0.5, 1e-6);                                 // metade da imagem cabe na tela
    CHECK_NEAR(j.y, -6.0, 1e-5);                                     // -(camY - refCamY) * taxaVertical

    // imagem mais baixa que a tela e esticada larguraDaCamada = w * (viewH / h)
    l.tex.w = 1000;
    l.tex.h = 270;
    j = l.computeWindow(0.0f, 20.0f, 20.0f, 0.0f, 960.0f, 540.0f);
    CHECK_NEAR(j.uScale, 960.0 / 2000.0, 1e-6);
    CHECK_NEAR(j.u0, 0.0, 1e-9);
    CHECK_NEAR(j.y, 0.0, 1e-9);
    return resultado();
}
