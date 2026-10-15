#define STB_IMAGE_IMPLEMENTATION
#include "Game.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef TESTE_REPLAY
#include "replay_common.h"
#endif

namespace
{
bool fileExists(const std::string& p) { std::ifstream f(p); return f.good(); }

void keyCallback(GLFWwindow* w, int key, int, int action, int)
{
    static_cast<Game*>(glfwGetWindowUserPointer(w))->onKey(key, action);
}
void scrollCallback(GLFWwindow* w, double, double dy)
{
    static_cast<Game*>(glfwGetWindowUserPointer(w))->onScroll(dy);
}
}

Game::~Game()
{
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

std::string Game::assetPath(const std::string& rel) const { return root_ + "/assets/" + rel; }

// Inicializacao config, janela, OpenGL, shaders, texturas, texto, camadas
bool Game::init()
{
    root_ = fileExists("assets/config.txt") ? "." : GAME_ROOT;
    if (!cfg_.load(assetPath("config.txt")))
    {
        std::cerr << "Nao encontrei assets/config.txt (procurei em '" << root_ << "')\n";
        return false;
    }

    int winW = cfg_.integer("window.width", 960), winH = cfg_.integer("window.height", 540);
    camera_.viewW = static_cast<float>(winW);
    camera_.viewH = static_cast<float>(winH);
    vsync_ = cfg_.integer("window.vsync", 1) != 0;

    if (!glfwInit()) { std::cerr << "Falha ao iniciar o GLFW\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef TESTE_REPLAY
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif

    window_ = glfwCreateWindow(winW, winH, cfg_.str("window.title", "Terraria :D").c_str(), nullptr, nullptr);
    if (!window_) { std::cerr << "Falha ao criar a janela (OpenGL 3.3 core)\n"; return false; }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(vsync_ ? 1 : 0);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, keyCallback);
    glfwSetScrollCallback(window_, scrollCallback);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))
    {
        std::cerr << "Falha ao carregar o OpenGL via GLAD\n";
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!renderer_.init(assetPath("shaders/sprite.vert"), assetPath("shaders/sprite.frag"))) return false;

    // texturas
    bool ok = tex_.tiles.load(assetPath("tiles.png")) && tex_.crack.load(assetPath("crack.png")) &&
              tex_.hearts.load(assetPath("hearts.png")) && tex_.slots.load(assetPath("slots.png")) &&
              tex_.items.load(assetPath("items.png")) &&
              tex_.player.load(assetPath(cfg_.str("player.sprite", "player.png"))) &&
              tex_.slime.load(assetPath(cfg_.str("slime.sprite", "slime.png"))) &&
              tex_.zombie.load(assetPath(cfg_.str("zombie.sprite", "zombie.png"))) &&
              tex_.drops.load(assetPath(cfg_.str("drops.sprite", "drops.png")));
    if (!ok) return false;
    tex_.white.makeWhite();

    // texto do HUD (FreeType) arquivo da fonte e tamanhos vem do config
    std::vector<int> sizes;
    for (float s : cfg_.numbers("font.sizes")) sizes.push_back(static_cast<int>(s));
    if (sizes.size() != 3) sizes = { 16, 32, 80 };
    if (!text_.init(assetPath("shaders/text.vert"), assetPath("shaders/text.frag"),
                    assetPath(cfg_.str("font.file", "fonts/DejaVuSansMono-Bold.ttf")), sizes)) return false;
    hud_.init(&renderer_, &text_, &tex_, sizes[0], sizes[1], sizes[2], camera_.viewW, camera_.viewH);

    // camadas de parallax definidas no config
    for (const std::string& line : cfg_.layers())
    {
        std::string s = line;
        for (char& c : s) if (c == ',') c = ' ';
        std::istringstream ss(s);
        std::string file, mode;
        Layer layer;
        ss >> file >> layer.rate >> layer.vrate >> layer.drift >> mode;
        if (!layer.tex.load(assetPath(file), true)) return false;
        layer.mode = mode == "sky" ? Layer::Mode::Sky : mode == "day" ? Layer::Mode::Day
                   : mode == "night" ? Layer::Mode::Night : Layer::Mode::Fixed;
        layers_.push_back(layer);
    }

    speed_ = cfg_.num("player.speed", 240);
    jump_ = cfg_.num("player.jump", 520);
    gravity_ = cfg_.num("player.gravity", 1500);
    reach_ = cfg_.num("player.reach", 160);
    regenTime_ = cfg_.num("player.regen", 12);
    fallSafe_ = cfg_.num("player.fall.safe", 192);
    fallPerHeart_ = cfg_.num("player.fall.perheart", 96);
    swordDamage_ = cfg_.integer("sword.damage", 2);
    punchDamage_ = cfg_.integer("punch.damage", 1);
    zombieSpeed_ = cfg_.num("zombie.speed", 75);
    dayNight_.length = cfg_.num("day.length", 120);
    dayNight_.start = cfg_.num("day.start", 0.12f);

    // valores dos dois tipos de inimigo
    slimeCfg_.hp = cfg_.integer("slime.hp", 4);
    slimeCfg_.damage = cfg_.integer("slime.damage", 1);
    slimeCfg_.nFrames = cfg_.integer("slime.cols", 4);
    slimeCfg_.nAnimations = cfg_.integer("slime.rows", 1);
    slimeCfg_.frames = cfg_.numbers("slime.frames");
    slimeCfg_.fps = cfg_.numbers("slime.fps");
    zombieCfg_.hp = cfg_.integer("zombie.hp", 6);
    zombieCfg_.damage = cfg_.integer("zombie.damage", 2);
    zombieCfg_.nFrames = cfg_.integer("zombie.cols", 4);
    zombieCfg_.nAnimations = cfg_.integer("zombie.rows", 1);
    zombieCfg_.frames = cfg_.numbers("zombie.frames");
    zombieCfg_.fps = cfg_.numbers("zombie.fps");

    reset();
    return true;
}

// Gera um mundo novo e coloca o jogador no centro sobre a grama
void Game::reset()
{
    world_.generate(cfg_.integer("world.width", 300), cfg_.integer("world.height", 100),
                    static_cast<unsigned>(cfg_.integer("world.seed", 1234)));

    int sx = world_.W / 2;
    player_ = Player();
    player_.body.pos = { sx * TILE + TILE * 0.5f, static_cast<float>(world_.topSolid(sx) * TILE) };
    player_.body.hw = 11.0f;    // hitbox 22x62 (o quad do sprite e 72x72)
    player_.body.h = 62.0f;
    player_.maxHp = player_.hp = cfg_.integer("player.hearts", 5);
    player_.apexY = player_.body.pos.y;
    player_.sprite.setup(&tex_.player, cfg_.integer("player.cols", 6), cfg_.integer("player.rows", 4),
                         cfg_.numbers("player.frames"), cfg_.numbers("player.fps"));
    player_.sprite.size = { 72.0f, 72.0f };

    enemies_.clear();
    drops_.clear();
    floatTexts_.clear();
    inventory_.reset();
    mineX_ = mineY_ = -1;
    mineProgress_ = 0.0f;
    kills_ = blocksMined_ = coins_ = 0;
    survived_ = 0.0f;
    spawnTimer_ = 4.0f;
    clock_ = 0.0f;

    updateCamera(0.0f, true);
    refCamY_ = camera_.pos.y;
}

// Laco principal (aula Camadas e Sprites) processInput -> update -> render -> sync
void Game::run()
{
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window_))
    {
        timer_.start();
        double now = glfwGetTime();
        float dt = std::min(static_cast<float>(now - last), 0.033f);   // limita para a fisica nao explodir
        last = now;

        glfwPollEvents();
        processInput();
        update(dt);
        render();

        timer_.finish();
        if (!vsync_)   // sem V-Sync espera o resto do quadro para ficar em 60 FPS
        {
            double wait = timer_.calcWaitingTime(60, timer_.getElapsedTimeMs());
            if (wait > 0.0) std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(wait)));
        }
        glfwSwapBuffers(window_);
    }
}

void Game::onKey(int key, int action)
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window_, GLFW_TRUE);
    if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
    {
        if (state_ != State::Playing)
        {
            reset();
            state_ = State::Playing;
        }
    }
    if (key >= GLFW_KEY_1 && key < GLFW_KEY_1 + Inventory::kSlots)
        inventory_.select(key - GLFW_KEY_1);
}

void Game::onScroll(double dy)
{
    if (dy == 0.0) return;
    inventory_.scroll(dy);
}

// le o teclado e o mouse uma vez por quadro e guarda no Input
void Game::processInput()
{
#ifdef TESTE_REPLAY
    if (g_replay.ativo)   // testes as entradas vem do roteiro
    {
        input_.moveX = g_replay.cur.moveX;
        input_.jump = g_replay.cur.pula;
        input_.mouseLeft = g_replay.cur.esq;
        input_.mouseRight = g_replay.cur.dir;
        input_.mouseWorld = mouseWorld();   // = g_replay.mouse (fixado no inicio do quadro)
        return;
    }
#endif
    input_.moveX = 0.0f;
    if (keyDown(GLFW_KEY_A) || keyDown(GLFW_KEY_LEFT)) input_.moveX -= 1.0f;
    if (keyDown(GLFW_KEY_D) || keyDown(GLFW_KEY_RIGHT)) input_.moveX += 1.0f;
    input_.jump = keyDown(GLFW_KEY_W) || keyDown(GLFW_KEY_UP) || keyDown(GLFW_KEY_SPACE);
    input_.mouseLeft = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    input_.mouseRight = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    input_.mouseWorld = mouseWorld();
}

// Posicao do mouse em coordenadas de mundo (tambem usada no desenho para o destaque do bloco)
glm::vec2 Game::mouseWorld() const
{
#ifdef TESTE_REPLAY
    if (g_replay.ativo) return g_replay.mouse;
#endif
    double mx, my;
    int ww, wh;
    glfwGetCursorPos(window_, &mx, &my);
    glfwGetWindowSize(window_, &ww, &wh);
    return camera_.screenToWorld(mx, my, ww, wh);
}
