#include "Game.h"

#include <algorithm>
#include <cmath>

namespace
{
const float kDegToRad = 3.14159265f / 180.0f;

// Cor de uma camada de fundo mistura a cor da noite com branco conforme a hora (d = 1 dia, 0 noite)
glm::vec4 corDaCamada(glm::vec3 noite, float d)
{
    return glm::vec4(noite + (glm::vec3(1.0f) - noite) * d, 1.0f);
}
}  // namespace

// ordem de desenho fundo (parallax), mundo e personagens, HUD
void Game::render()
{
    int fbw, fbh;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();
    drawWorld();
    drawHud();
}

void Game::drawBackground()
{
    renderer_.setProjection(0.0f, camera_.viewW, camera_.viewH, 0.0f);   // camadas usam a projecao de tela (fixa)
    float d = dayNight_.factor(clock_);

    for (const Layer& l : layers_)
    {
        glm::vec4 tint(1.0f);
        switch (l.mode)
        {
        case Layer::Mode::Sky:   tint = corDaCamada({ 0.04f, 0.06f, 0.16f }, d); break;
        case Layer::Mode::Day:   tint = corDaCamada({ 0.22f, 0.26f, 0.48f }, d); break;
        case Layer::Mode::Night: tint = { 1.0f, 1.0f, 1.0f, std::min(1.0f, (1.0f - d) * 1.4f) }; break;
        case Layer::Mode::Fixed: break;
        }
        l.draw(renderer_, camera_.pos.x, camera_.pos.y, refCamY_, clock_, camera_.viewW, camera_.viewH, tint);
    }
}

void Game::drawWorld()
{
    renderer_.setProjection(camera_.left(), camera_.right(), camera_.bottom(), camera_.top());   // projecao segue a camera

    int x0 = static_cast<int>(std::floor(camera_.left() / TILE)), x1 = static_cast<int>(std::floor(camera_.right() / TILE));
    int y0 = static_cast<int>(std::floor(camera_.top() / TILE)), y1 = static_cast<int>(std::floor(camera_.bottom() / TILE));
    const float dx = 1.0f / ATLAS_TILES;

    for (int ty = y0; ty <= y1; ++ty)
        for (int tx = x0; tx <= x1; ++tx)
        {
            uint8_t t = world_.get(tx, ty);
            glm::vec2 pos(float(tx * TILE), float(ty * TILE));
            glm::vec4 light = dayNight_.lightAt(clock_, world_, tx, ty);
            if (t == AIR)
            {
                if (world_.underground(tx, ty))   // parede escura da caverna
                    renderer_.draw(tex_.white, pos, glm::vec2(TILE), {0, 0}, {1, 1}, light * glm::vec4(0.38f, 0.28f, 0.23f, 1.0f));
                continue;
            }
            renderer_.draw(tex_.tiles, pos, glm::vec2(TILE), { atlasIndex(t) * dx, 0.0f }, { dx, 1.0f }, light);
        }

    if (state_ == State::Title) return;

    // rachadura do bloco sendo minerado (4 quadros)
    if (mineX_ >= 0 && mineProgress_ > 0.0f)
    {
        int f = std::min(3, static_cast<int>(mineProgress_ * 4.0f));
        renderer_.draw(tex_.crack, { float(mineX_ * TILE), float(mineY_ * TILE) }, glm::vec2(TILE), { f * 0.25f, 0.0f }, { 0.25f, 1.0f });
    }

    // destaque do bloco sob o mouse (so se estiver ao alcance e sem a espada na mao)
    glm::vec2 m = mouseWorld();
    int hx = static_cast<int>(std::floor(m.x / TILE)), hy = static_cast<int>(std::floor(m.y / TILE));
    if (state_ == State::Playing && !inventory_.swordSelected() &&
        glm::length(glm::vec2((hx + 0.5f) * TILE, (hy + 0.5f) * TILE) - playerCenter()) <= reach_)
        renderer_.draw(tex_.white, { float(hx * TILE), float(hy * TILE) }, glm::vec2(TILE), { 0, 0 }, { 1, 1 }, { 1.0f, 1.0f, 1.0f, 0.22f });

    // moedas e coracoes
    for (const Drop& d : drops_)
    {
        glm::vec4 tint = dayNight_.lightAt(clock_, world_, static_cast<int>(d.body.pos.x / TILE), static_cast<int>((d.body.pos.y - 1) / TILE));
        if (d.life < 4.0f && std::fmod(d.life, 0.4f) < 0.2f) continue;   // pisca antes de sumir
        d.sprite.draw(renderer_, { d.body.pos.x - 12.0f, d.body.pos.y - 24.0f }, tint);
    }

    // inimigos (com barra de vida quando machucados)
    for (const Enemy& e : enemies_)
    {
        glm::vec4 tint = dayNight_.lightAt(clock_, world_, static_cast<int>(e.body.pos.x / TILE), static_cast<int>((e.body.pos.y - 1) / TILE));
        if (e.flash > 0.0f) tint = { 1.0f, 0.35f, 0.35f, 1.0f };
        float half = e.sprite.size.x * 0.5f;
        e.sprite.draw(renderer_, { e.body.pos.x - half, e.body.pos.y - e.sprite.size.y }, tint);

        if (e.hp < e.maxHp)
        {
            float ratio = std::max(0.0f, static_cast<float>(e.hp) / e.maxHp);
            glm::vec2 bar(e.body.pos.x - 16.0f, e.body.pos.y - e.body.h - 10.0f);
            renderer_.draw(tex_.white, bar - glm::vec2(1.0f), { 34.0f, 6.0f }, {0, 0}, {1, 1}, { 0.0f, 0.0f, 0.0f, 0.75f });
            renderer_.draw(tex_.white, bar, { 32.0f * ratio, 4.0f }, {0, 0}, {1, 1}, { 1.0f - ratio, 0.3f + 0.6f * ratio, 0.25f, 1.0f });
        }
    }

    // jogador (pisca enquanto esta invulneravel)
    bool blink = player_.invuln > 0.0f && std::fmod(player_.invuln, 0.2f) < 0.1f;
    if (!blink)
    {
        glm::vec2 c = playerCenter();
        glm::vec4 tint = dayNight_.lightAt(clock_, world_, static_cast<int>(c.x / TILE), static_cast<int>(c.y / TILE));
        player_.sprite.draw(renderer_, { player_.body.pos.x - 36.0f, player_.body.pos.y - 72.0f }, tint);

        // espada na mao gira em torno do cabo (matriz de modelo com rotacao)
        if (inventory_.swordSelected())
        {
            float f = player_.swinging ? static_cast<float>(player_.swingFacing) : static_cast<float>(player_.facing);
            float angle = 25.0f;   // parada levantada na frente do corpo
            if (player_.swinging)
            {
                float p = std::min(1.0f, player_.swingT / Player::kSwingDuration);
                angle = -80.0f + 195.0f * p * (2.0f - p);   // arco de tras/cima ate a frente/baixo, desacelerando
            }
            glm::vec2 hand(player_.body.pos.x + f * 8.0f, player_.body.pos.y - 30.0f);
            renderer_.drawRotated(tex_.items, hand, { 48.0f, 48.0f }, { 0.5f, 0.85f }, f * angle * kDegToRad,
                                  { 0.5f, 0.0f }, { 0.5f, 1.0f }, tint, f < 0.0f);
        }
    }

    // numeros de dano / cura subindo (texto em coordenadas de mundo)
    hud_.drawFloatTexts(floatTexts_, camera_);
}

void Game::drawHud()
{
    renderer_.setProjection(0.0f, camera_.viewW, camera_.viewH, 0.0f);
    hud_.beginScreen();

    if (state_ == State::Title)
    {
        hud_.drawTitle(clock_);
        return;
    }

    HudInfo info;
    info.inventory = &inventory_;
    info.hp = player_.hp;
    info.maxHp = player_.maxHp;
    info.coins = coins_;
    info.kills = kills_;
    info.blocksMined = blocksMined_;
    int tx = static_cast<int>(playerCenter().x / TILE), ty = static_cast<int>(playerCenter().y / TILE);
    info.depth = std::max(0, ty - world_.surface(tx));
    info.isDay = dayNight_.factor(clock_) > 0.5f;
    info.survived = survived_;

    hud_.drawPlaying(info);
    if (state_ == State::GameOver) hud_.drawGameOver(info, clock_);
}
