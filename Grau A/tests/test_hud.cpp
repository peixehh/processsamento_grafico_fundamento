#define STB_IMAGE_IMPLEMENTATION
#include "check.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "Hud.h"

static const int kW = 960, kH = 540;

// Quantos pixels do retangulo [x0,x1) x [y0,y1) (y medido de cima para baixo, como o HUD) sao diferentes
// da cor (r,g,b), com folga de 3 por canal por causa do arredondamento da mistura.
static int mudados(const std::vector<unsigned char>& px, int x0, int y0, int x1, int y1,
                   int r = 51, int g = 102, int b = 153)   // padrao o fundo (0.2, 0.4, 0.6)
{
    int n = 0;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
        {
            const unsigned char* p = &px[(static_cast<size_t>(kH - 1 - y) * kW + x) * 4];
            if (std::abs(p[0] - r) > 3 || std::abs(p[1] - g) > 3 || std::abs(p[2] - b) > 3) ++n;
        }
    return n;
}

static std::vector<unsigned char> quadro()
{
    std::vector<unsigned char> px(static_cast<size_t>(kW) * kH * 4);
    glReadPixels(0, 0, kW, kH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    return px;
}

static void limpa()
{
    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main()
{
    if (!glfwInit()) return 2;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* janela = glfwCreateWindow(kW, kH, "teste", nullptr, nullptr);
    if (!janela) { glfwTerminate(); return 2; }
    glfwMakeContextCurrent(janela);
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) return 2;

    const std::string base = std::string(GAME_ROOT) + "/assets/";
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SpriteRenderer renderer;
    CHECK(renderer.init(base + "shaders/sprite.vert", base + "shaders/sprite.frag"));
    TextRenderer text;
    CHECK(text.init(base + "shaders/text.vert", base + "shaders/text.frag", base + "fonts/DejaVuSansMono-Bold.ttf", { 16, 32, 80 }));
    Texturas tex;
    CHECK(tex.tiles.load(base + "tiles.png") && tex.hearts.load(base + "hearts.png") && tex.slots.load(base + "slots.png") &&
          tex.items.load(base + "items.png") && tex.drops.load(base + "drops.png"));
    tex.white.makeWhite();

    Hud hud;
    hud.init(&renderer, &text, &tex, 16, 32, 80, static_cast<float>(kW), static_cast<float>(kH));

    // framebuffer fora da tela
    GLuint fbo = 0, cor = 0;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &cor);
    glBindTexture(GL_TEXTURE_2D, cor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kW, kH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cor, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glViewport(0, 0, kW, kH);

    Inventory inv;
    for (int i = 0; i < 12; ++i) inv.give(DIRT);
    inv.select(2);
    HudInfo info;
    info.inventory = &inv;
    info.hp = 3;
    info.maxHp = 5;
    info.coins = 7;
    info.kills = 2;
    info.blocksMined = 9;
    info.depth = 12;
    info.isDay = false;
    info.survived = 75.0f;

    glGetError();

    // 1) titulo escurece tudo e escreve o nome do jogo no alto
    limpa();
    renderer.setProjection(0.0f, kW, kH, 0.0f);
    hud.beginScreen();
    hud.drawTitle(0.0f);
    CHECK(glGetError() == GL_NO_ERROR);
    std::vector<unsigned char> px = quadro();
    CHECK(px[((kH - 1 - 5) * kW + 5) * 4] < 51);               // canto escurecido (0.45 de preto)
    // o veu deixa o fundo em (28, 56, 84), o nome do jogo (fonte de 80 px, amarelo) aparece por cima
    CHECK(mudados(px, 200, 100, 760, 190, 28, 56, 84) > 1500);
    CHECK(mudados(px, 0, 405, 960, 440, 28, 56, 84) == 0);     // a antiga dica "Cuidado com quedas altas..." (y = 415) saiu
    CHECK(mudados(px, 0, 440, 960, 540, 28, 56, 84) == 0);     // embaixo nao ha texto so o veu

    // 2) jogo barra de itens (esq.), coracoes e contadores (dir.), sem escurecer o resto
    limpa();
    renderer.setProjection(0.0f, kW, kH, 0.0f);
    hud.beginScreen();
    hud.drawPlaying(info);
    CHECK(glGetError() == GL_NO_ERROR);
    px = quadro();
    CHECK(mudados(px, 14, 12, 400, 54) > 3000);                // 8 espacos da barra
    CHECK(mudados(px, 780, 12, 950, 46) > 300);                // 5 coracoes
    CHECK(mudados(px, 830, 48, 958, 124) > 3000);              // caixa dos contadores
    CHECK(mudados(px, 300, 300, 600, 400) == 0);               // o meio da tela nao foi tocado
    CHECK(px[((kH - 1 - 300) * kW + 300) * 4] == 51);

    // 3) game over veu vermelho por cima de tudo
    renderer.setProjection(0.0f, kW, kH, 0.0f);
    hud.beginScreen();
    hud.drawGameOver(info, 0.0f);
    CHECK(glGetError() == GL_NO_ERROR);
    px = quadro();
    const unsigned char* c = &px[((kH - 1 - 300) * kW + 300) * 4];
    CHECK(c[0] > 51 && c[1] < 102);                            // mais vermelho e menos verde que o fundo

    // 4) numeros flutuantes em coordenadas de mundo seguem a camera
    limpa();
    Camera cam;
    cam.pos = { 1000.0f, 500.0f };
    std::vector<FloatText> textos(1);
    textos[0].text = "-3";
    textos[0].pos = { 1100.0f, 600.0f };
    textos[0].color = { 1.0f, 0.25f, 0.25f, 1.0f };
    textos[0].life = textos[0].maxLife = 0.9f;
    hud.drawFloatTexts(textos, cam);
    CHECK(glGetError() == GL_NO_ERROR);
    px = quadro();
    CHECK(mudados(px, 90, 90, 140, 130) > 20);                 // (1100,600) - (1000,500) = (100,100) na tela

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &cor);
    glfwDestroyWindow(janela);
    glfwTerminate();
    return resultado();
}
