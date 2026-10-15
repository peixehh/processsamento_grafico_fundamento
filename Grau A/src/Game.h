#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <random>
#include <string>
#include <vector>

#include "Camera.h"
#include "Config.h"
#include "DayNight.h"
#include "Drop.h"
#include "Enemy.h"
#include "Hud.h"
#include "Input.h"
#include "Inventory.h"
#include "Layer.h"
#include "Player.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "Texturas.h"
#include "Timer.h"
#include "World.h"

// Laco principal e regras do jogo. A implementacao fica em tres arquivos, um por fase do laco da aula
//   Game.cpp        init, reset, run e processInput (fase 1)
//   GameUpdate.cpp  update e as regras dano, drops, mineracao, espada, inimigos (fase 2)
//   GameRender.cpp  render fundo, mundo e HUD (fase 3)
class Game
{
public:
    bool init();
    void run();
    ~Game();

    // chamados pelos callbacks do GLFW
    void onKey(int key, int action);
    void onScroll(double dy);

#ifdef TESTE_REPLAY
    void runReplay(const char* tracePath, const char* shotPrefix);   // so existe nos testes (tests/replay_game.cpp)
#endif

private:
    enum class State { Title, Playing, GameOver };

    // inicializacao / mundo
    std::string assetPath(const std::string& rel) const;
    void reset();

    // fase 1 entrada (Game.cpp)
    void processInput();
    bool keyDown(int key) const { return glfwGetKey(window_, key) == GLFW_PRESS; }
    glm::vec2 mouseWorld() const;

    // fase 2 atualizacao (GameUpdate.cpp)
    void update(float dt);
    void updatePlayer(float dt);
    void updateMiningAndPlacing(float dt);
    void updateSword(float dt);
    void updateEnemies(float dt);
    void updateDrops(float dt);
    void updateFloatTexts(float dt);
    void updateCamera(float dt, bool snap);
    void spawnEnemy();
    void spawnDrop(Drop::Kind kind, glm::vec2 pos);
    void breakTile(int tx, int ty);
    void damageEnemy(Enemy& e, int dmg, int fromDir);
    void onEnemyDeath(const Enemy& e);
    void damagePlayer(int amount, int fromDir, bool knockback);
    void onPlayerHurt(int amount);
    void addFloatText(const std::string& text, glm::vec2 pos, glm::vec4 color);

    // fase 3 desenho (GameRender.cpp)
    void render();
    void drawBackground();
    void drawWorld();
    void drawHud();

    glm::vec2 playerCenter() const { return player_.center(); }

    GLFWwindow* window_ = nullptr;
    Config cfg_;
    std::string root_;
    bool vsync_ = true;

    Timer timer_;
    Input input_;
    Camera camera_;
    DayNight dayNight_;
    SpriteRenderer renderer_;
    TextRenderer text_;
    Hud hud_;
    Texturas tex_;
    std::vector<Layer> layers_;

    World world_;
    Player player_;
    Inventory inventory_;
    std::vector<Enemy> enemies_;
    std::vector<Drop> drops_;
    std::vector<FloatText> floatTexts_;
    EnemyConfig slimeCfg_, zombieCfg_;
    std::mt19937 rng_{12345};

    State state_ = State::Title;
    float clock_ = 0.0f;       // tempo total (anima o ciclo dia/noite e o parallax)
    float refCamY_ = 0.0f;     // altura da camera no spawn (referencia do parallax vertical)

    // mineracao
    int mineX_ = -1, mineY_ = -1;
    float mineProgress_ = 0.0f;
    float placeCooldown_ = 0.0f, attackCooldown_ = 0.0f;

    // inimigos / progresso
    float spawnTimer_ = 3.0f;
    int nextEnemyId_ = 1;
    int kills_ = 0, blocksMined_ = 0, coins_ = 0;
    float survived_ = 0.0f;

    // parametros lidos do config
    float speed_ = 240, jump_ = 520, gravity_ = 1500, reach_ = 160, regenTime_ = 12;
    float fallSafe_ = 192, fallPerHeart_ = 96;
    int swordDamage_ = 2, punchDamage_ = 1;
    float zombieSpeed_ = 75;
};
