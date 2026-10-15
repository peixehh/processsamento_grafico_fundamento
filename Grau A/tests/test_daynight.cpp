#include "check.h"
#include "DayNight.h"
#include "World.h"

int main()
{
    DayNight dn;   // length = 120 s e start = 0.12, os valores do config.txt
    CHECK_NEAR(dn.factor(0.0f), 1.0, 1e-5);
    CHECK_NEAR(dn.factor(20.0f), 1.0, 1e-5);
    CHECK_NEAR(dn.factor(47.0f), 0.441409, 1e-4);
    CHECK_NEAR(dn.factor(60.0f), 0.0, 1e-5);
    CHECK_NEAR(dn.factor(100.0f), 0.268775, 1e-4);
    CHECK_NEAR(dn.factor(120.0f), dn.factor(0.0f), 1e-4);   // o ciclo fecha em 'length' segundos

    DayNight lento;
    lento.length = 240.0f;
    lento.start = 0.0f;
    CHECK_NEAR(lento.factor(60.0f), 1.0, 1e-5);     // fase 0.25 meio-dia (limitado em 1)
    CHECK_NEAR(lento.factor(180.0f), 0.0, 1e-5);    // fase 0.75 meia-noite

    // iluminacao depende da hora e da profundidade abaixo da superficie (em blocos)
    World w;
    w.generate(300, 100, 1234);
    int sf = w.surface(150);
    glm::vec4 c = dn.lightAt(0.0f, w, 150, sf);              // dia, na superficie branco
    CHECK_NEAR(c.r, 1.0, 1e-4);
    CHECK_NEAR(c.g, 1.0, 1e-4);
    CHECK_NEAR(c.b, 1.0, 1e-4);
    CHECK_NEAR(c.a, 1.0, 1e-6);
    c = dn.lightAt(0.0f, w, 150, sf + 8);                    // dia, 8 blocos abaixo mais escuro
    CHECK_NEAR(c.r, 0.7, 1e-4);
    CHECK_NEAR(c.g, 0.7, 1e-4);
    CHECK_NEAR(c.b, 0.7, 1e-4);
    c = dn.lightAt(60.0f, w, 150, sf);                       // noite, na superficie azulada
    CHECK_NEAR(c.r, 0.288, 1e-4);
    CHECK_NEAR(c.g, 0.320, 1e-4);
    CHECK_NEAR(c.b, 0.460, 1e-4);
    c = dn.lightAt(60.0f, w, 150, sf + 8);
    CHECK_NEAR(c.r, 0.344, 1e-4);
    CHECK_NEAR(c.g, 0.360, 1e-4);
    CHECK_NEAR(c.b, 0.430, 1e-4);
    c = dn.lightAt(60.0f, w, 150, sf + 16);                  // 16 blocos caverna, cinza escuro
    CHECK_NEAR(c.r, 0.4, 1e-4);
    CHECK_NEAR(c.b, 0.4, 1e-4);
    c = dn.lightAt(60.0f, w, 150, sf + 40);                  // mais fundo que isso a profundidade e limitada
    CHECK_NEAR(c.r, 0.4, 1e-4);
    CHECK_NEAR(c.b, 0.4, 1e-4);
    c = dn.lightAt(47.0f, w, 150, sf + 4);                   // entardecer, perto da superficie
    CHECK_NEAR(c.r, 0.528412, 1e-4);
    CHECK_NEAR(c.g, 0.548475, 1e-4);
    CHECK_NEAR(c.b, 0.636253, 1e-4);
    c = dn.lightAt(0.0f, w, 150, sf - 5);                    // acima da superficie profundidade negativa vira 0
    CHECK_NEAR(c.r, 1.0, 1e-4);
    return resultado();
}
