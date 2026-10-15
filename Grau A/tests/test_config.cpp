#include "check.h"
#include "Config.h"

#include <string>
#include <vector>

int main()
{
    Config c;
    CHECK(c.load(std::string(GAME_ROOT) + "/assets/config.txt"));
    CHECK_NEAR(c.num("player.speed", -1), 240, 1e-6);
    CHECK(c.integer("world.width", -1) == 300);
    CHECK(c.str("window.title", "") == "Terraria :D");
    std::vector<float> frames = c.numbers("player.frames");
    CHECK(frames.size() == 4 && frames[0] == 4 && frames[1] == 6 && frames[2] == 2 && frames[3] == 4);
    CHECK(c.layers().size() == 6);

    // chaves ausentes valor padrao / lista vazia (o jogo usa isso para font.sizes, window.vsync...)
    CHECK_NEAR(c.num("nao.existe", 7.5f), 7.5, 1e-6);
    CHECK(c.numbers("nao.existe").empty());
    CHECK(c.str("nao.existe", "padrao") == "padrao");

    // chaves novas do texto (FreeType) e do V-Sync
    CHECK(c.str("font.file", "") == "fonts/DejaVuSansMono-Bold.ttf");
    std::vector<float> tamanhos = c.numbers("font.sizes");
    CHECK(tamanhos.size() == 3 && tamanhos[0] == 16 && tamanhos[1] == 32 && tamanhos[2] == 80);
    CHECK(c.integer("window.vsync", -1) == 1);

    Config vazio;
    CHECK(!vazio.load(std::string(GAME_ROOT) + "/nao-existe.txt"));
    return resultado();
}
