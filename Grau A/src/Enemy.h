#pragma once
#include <random>
#include <vector>

#include <glm/glm.hpp>

#include "Body.h"
#include "Sprite.h"
#include "World.h"

// Valores de um tipo de inimigo (vem do config.txt)
struct EnemyConfig
{
    int hp = 3, damage = 1;
    int nFrames = 4, nAnimations = 1;       // colunas e linhas do spritesheet
    std::vector<float> frames, fps;         // quadros e fps de cada animacao
};

// O que a IA precisa saber do resto do jogo
struct EnemyEnv
{
    const World& world;
    std::mt19937& rng;
    glm::vec2 playerPos;
    float gravity, zombieSpeed;
    float clock;               // tempo do jogo (anima o slime respirando)
};

// Inimigos slime (pula) e zumbi (anda). Mesma estrutura, comportamento escolhido pelo tipo.
struct Enemy
{
    enum class Type { Slime, Zombie };
    Type type = Type::Slime;
    int id = 0;
    Body body;
    Sprite sprite;
    int hp = 3, maxHp = 3;
    int damage = 1;           // coracoes tirados ao encostar no jogador
    float hopTimer = 0.5f;    // slime tempo ate o proximo pulo | zumbi tempo ate trocar de direcao
    float flash = 0.0f;       // pisca vermelho ao ser atingido
    float stun = 0.0f;        // apos o golpe nao controla o proprio movimento
    int dir = 1;

    // pos = centro da base dos pes, dir ja nasce virado para o jogador. Sorteia os temporizadores iniciais.
    static Enemy makeSlime(glm::vec2 pos, int dir, int id, const Texture* tex, const EnemyConfig& cfg, std::mt19937& rng);
    static Enemy makeZombie(glm::vec2 pos, int dir, int id, const Texture* tex, const EnemyConfig& cfg, std::mt19937& rng);

    // Longe demais, ou zumbi fora da tela quando amanhece.
    bool shouldDespawn(float playerX, float dayFactor) const;

    // IA, fisica e animacao de um quadro.
    void update(float dt, const EnemyEnv& env);
};
