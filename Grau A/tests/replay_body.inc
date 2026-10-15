// Corpo de Game::runReplay. Este arquivo e incluido (1) no fim do Game.cpp da copia do codigo
// original e (2) em tests/replay_game.cpp, para a versao refatorada. Quem inclui define antes
//   REPLAY_SELECT(i)        escolhe o item i da barra
//   REPLAY_GIVE_BLOCKS()    deixa 30 blocos em cada espaco de bloco (2 a 7)
//   REPLAY_PROCESS_INPUT()  le a entrada do quadro (vazio no original, processInput() na versao nova)
// O resto so usa membros que existem nas duas versoes.

void Game::runReplay(const char* tracePath, const char* shotPrefix)
{
    const float dt = 1.0f / 60.0f;
    FILE* trace = std::fopen(tracePath, "w");
    if (!trace) return;
    std::fprintf(trace, "frame,px,py,hp,invuln,estado,inimigos,somaEx,somaHp,drops,moedas,mortes,blocos,mundo\n");

    // 1) tela de titulo a camera passeia por 90 quadros
    for (int i = 0; i < 90; ++i) update(dt);
    render();
    replayImagem(window_, shotPrefix, "titulo");

    // 2) comeca a partida como o ENTER faz e solta o jogador de 12 blocos de altura
    reset();
    state_ = State::Playing;
    player_.body.pos.y -= 400.0f;
    player_.apexY = player_.body.pos.y;
    REPLAY_GIVE_BLOCKS();

    for (int f = 0; f < kReplayFrames; ++f)
    {
        g_replay.cur = replayPasso(f);
        // Mantem o jogador vivo para o roteiro inteiro rodar (cura a cada 100 quadros) e, perto do fim, faz o
        // jogador morrer pelo caminho real (damagePlayer), cobrindo tambem a passagem para o game over.
        if (f % 100 == 99 && player_.hp > 0) player_.hp = player_.maxHp;
        if (f == 4790)
        {
            player_.invuln = 0.0f;
            damagePlayer(99, 1, true);
        }
        if (g_replay.cur.mira && !enemies_.empty())
        {
            // aponta o mouse para o inimigo mais proximo (centro do jogador -> centro do inimigo)
            glm::vec2 c = player_.body.pos - glm::vec2(0.0f, player_.body.h * 0.5f);
            float melhor = 1e30f;
            for (const Enemy& e : enemies_)
            {
                glm::vec2 d = (e.body.pos - glm::vec2(0.0f, e.body.h * 0.5f)) - c;
                float dist = d.x * d.x + d.y * d.y;
                if (dist < melhor)
                {
                    melhor = dist;
                    g_replay.cur.offX = d.x;
                    g_replay.cur.offY = d.y;
                }
            }
        }
        // O mouse e fixado aqui, antes de qualquer atualizacao do quadro. (O original le o mouse DEPOIS de mover
        // o jogador e a versao nova ANTES, entao o deslocamento relativo ao jogador nao pode ser calculado la dentro.)
        g_replay.mouse = (player_.body.pos - glm::vec2(0.0f, player_.body.h * 0.5f)) +
                         glm::vec2(g_replay.cur.offX, g_replay.cur.offY);
        REPLAY_SELECT(g_replay.cur.slot);
        REPLAY_PROCESS_INPUT();
        update(dt);

        if (f % 10 == 0)
        {
            float somaEx = 0.0f;
            int somaHp = 0;
            for (const Enemy& e : enemies_)
            {
                somaEx += e.body.pos.x;
                somaHp += e.hp;
            }
            std::fprintf(trace, "%d,%.4f,%.4f,%d,%.4f,%d,%d,%.4f,%d,%d,%d,%d,%d,%016llx\n", f,
                         player_.body.pos.x, player_.body.pos.y, player_.hp, player_.invuln,
                         static_cast<int>(state_), static_cast<int>(enemies_.size()), somaEx, somaHp,
                         static_cast<int>(drops_.size()), coins_, kills_, blocksMined_,
                         static_cast<unsigned long long>(replayHashMundo(world_)));
        }
        if (f == 600 || f == 3300)
        {
            render();
            replayImagem(window_, shotPrefix, f == 600 ? "dia" : "noite");
        }
    }

    // 3) game over (o jogador morreu no quadro 4790)
    update(dt);
    render();
    replayImagem(window_, shotPrefix, "gameover");
    std::fclose(trace);
}
