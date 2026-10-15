#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "AABB.h"
#include "Body.h"
#include "Input.h"
#include "Sprite.h"
#include "World.h"

// Parametros que vem do config.txt
struct PlayerParams
{
    float speed = 240.0f;
    float jump = 520.0f;
    float gravity = 1500.0f;
    float fallSafe = 192.0f;       // queda (px) sem dano 6 blocos
    float fallPerHeart = 96.0f;    // a cada 96 px alem disso, perde mais 1 coracao
    float regenTime = 12.0f;       // segundos sem dano para recuperar 1 coracao
};

// O que aconteceu no quadro e o Game precisa tratar (numero de dano)
struct PlayerEvents
{
    int fallDamage = 0;   // coracoes perdidos ao pousar (0 = nenhum)
};

struct Player
{
    static constexpr float kSwingDuration = 0.28f;   // duracao do golpe da espada

    Body body;
    Sprite sprite;
    int facing = 1;           // 1 = direita, -1 = esquerda
    int hp = 5, maxHp = 5;
    float invuln = 0.0f;      // tempo de invulnerabilidade apos levar dano
    float regen = 0.0f;       // acumula tempo sem dano para recuperar 1 coracao
    bool mining = false;
    bool wasGround = true;    // para detectar o momento do pouso (dano de queda)
    float apexY = 0.0f;       // ponto mais alto desde que saiu do chao

    // golpe da espada
    bool swinging = false;
    float swingT = 0.0f, swingCooldown = 0.0f;
    int swingFacing = 1;
    std::vector<int> swingHits;   // inimigos ja atingidos neste golpe

    glm::vec2 center() const { return body.pos - glm::vec2(0.0f, body.h * 0.5f); }

    // Movimento horizontal, pulo, gravidade, colisao, dano de queda, invulnerabilidade e regeneracao.
    PlayerEvents update(float dt, const Input& in, const World& world, const PlayerParams& p);
    // Parte "fisica" do dano. Devolve false se um golpe de inimigo foi ignorado (invulneravel).
    bool takeDamage(int amount, int fromDir, bool knockback);
    // Escolhe a animacao do spritesheet pelo estado (minerando, no ar, andando ou parado).
    void animate(float dt);

    bool startSwing(int dir);                 // false se ja esta golpeando ou em recarga
    void updateSword(float dt);               // avanca o golpe, termina em kSwingDuration
    bool swordHitboxActive() const;           // entre 20% e 90% do golpe
    AABB swordHitbox() const;                 // caixa na frente do jogador
    bool alreadyHit(int enemyId) const;
    void registerHit(int enemyId) { swingHits.push_back(enemyId); }
};
