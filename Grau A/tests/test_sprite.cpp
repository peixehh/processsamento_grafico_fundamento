#include "check.h"
#include "golden.h"
#include "Sprite.h"

int main()
{
    // spritesheet do jogador 6 colunas x 4 linhas, quadros por animacao 4 6 2 4, fps 4 12 0 10
    Sprite s;
    s.setup(nullptr, 6, 4, { 4, 6, 2, 4 }, { 4, 12, 0, 10 });
    CHECK(s.nFrames == 6);
    CHECK(s.nAnimations == 4);
    CHECK(s.iAnimation == 0 && s.iFrame == 0);

    // animacao 1 (andando, 12 fps) a 60 quadros por segundo mesma sequencia do codigo original
    s.setAnim(1);
    for (int i = 0; i < 30; ++i)
    {
        s.update(1.0f / 60.0f);
        CHECK(s.iFrame == kGoldenFrames[i]);
    }

    // repetir a mesma animacao nao reinicia, trocar de animacao volta ao primeiro quadro
    int antes = s.iFrame;
    s.setAnim(1);
    CHECK(s.iFrame == antes);
    s.setAnim(2);
    CHECK(s.iAnimation == 2 && s.iFrame == 0);

    // fps 0 = animacao controlada a mao update nao anda
    for (int i = 0; i < 100; ++i) s.update(1.0f / 60.0f);
    CHECK(s.iFrame == 0);

    // listas curtas sao completadas (1 quadro, fps 0)
    Sprite t;
    t.setup(nullptr, 4, 3, { 4 }, { 8 });
    t.setAnim(2);
    for (int i = 0; i < 100; ++i) t.update(1.0f / 60.0f);
    CHECK(t.iFrame == 0);
    return resultado();
}
