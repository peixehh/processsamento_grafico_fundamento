// Executavel do rastro janela escondida, roteiro fixo, grava o rastro (.csv) e as imagens (.ppm).
// uso replay <rastro.csv> <prefixo-das-imagens>
#include <cstdio>

#include "Game.h"
#include "replay_common.h"

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::puts("uso: replay <rastro.csv> <prefixo-das-imagens>");
        return 2;
    }
    g_replay.ativo = true;
    Game game;
    if (!game.init()) return 1;
    game.runReplay(argv[1], argv[2]);
    return 0;
}
