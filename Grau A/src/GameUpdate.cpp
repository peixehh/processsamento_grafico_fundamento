#include "Game.h"

#include <algorithm>
#include <cmath>

// fisica, IA, colisoes e regras do jogo
void Game::update(float dt)
{
    clock_ += dt;

    if (state_ == State::Title)
    {
        // camera passeia pelo cenario para mostrar o parallax na tela inicial
        camera_.pos.x = world_.W * TILE * 0.5f - camera_.viewW * 0.5f + 500.0f * std::sin(clock_ * 0.2f);
        camera_.pos.y = refCamY_;
        return;
    }
    if (state_ == State::GameOver)
    {
        updateFloatTexts(dt);
        return;
    }

    survived_ += dt;
    updatePlayer(dt);
    updateMiningAndPlacing(dt);
    updateSword(dt);
    updateEnemies(dt);
    updateDrops(dt);
    updateFloatTexts(dt);
    updateCamera(dt, false);
    player_.animate(dt);   // escolhe a animacao do jogador
}

void Game::updateCamera(float dt, bool snap)
{
    camera_.follow(playerCenter(), dt, snap);
    camera_.clamp(static_cast<float>(world_.W * TILE), static_cast<float>(world_.H * TILE));
}

void Game::updatePlayer(float dt)
{
    PlayerParams params;
    params.speed = speed_;
    params.jump = jump_;
    params.gravity = gravity_;
    params.fallSafe = fallSafe_;
    params.fallPerHeart = fallPerHeart_;
    params.regenTime = regenTime_;

    PlayerEvents ev = player_.update(dt, input_, world_, params);
    if (ev.fallDamage > 0) onPlayerHurt(ev.fallDamage);
}

void Game::addFloatText(const std::string& text, glm::vec2 pos, glm::vec4 color)
{
    FloatText f;
    f.text = text;
    f.pos = pos;
    f.color = color;
    floatTexts_.push_back(f);
}

void Game::updateFloatTexts(float dt)
{
    for (size_t i = 0; i < floatTexts_.size();)
    {
        floatTexts_[i].life -= dt;
        floatTexts_[i].pos.y -= 45.0f * dt;
        if (floatTexts_[i].life <= 0.0f) floatTexts_.erase(floatTexts_.begin() + i);
        else ++i;
    }
}

// Golpe de inimigo (com recuo). A parte "fisica" fica no Player. O dano de queda NAO passa por aqui
// Player::update chama takeDamage e updatePlayer chama onPlayerHurt.
void Game::damagePlayer(int amount, int fromDir, bool knockback)
{
    if (!player_.takeDamage(amount, fromDir, knockback)) return;
    onPlayerHurt(amount);
}

// numero vermelho e fim de jogo
void Game::onPlayerHurt(int amount)
{
    addFloatText("-" + std::to_string(amount), { player_.body.pos.x - 10.0f, player_.body.pos.y - player_.body.h - 14.0f },
                 { 1.0f, 0.25f, 0.25f, 1.0f });
    if (player_.hp <= 0) state_ = State::GameOver;
}

void Game::damageEnemy(Enemy& e, int dmg, int fromDir)
{
    e.hp -= dmg;
    e.flash = 0.15f;
    e.stun = 0.3f;
    e.body.vel = { fromDir * 260.0f, -230.0f };
    addFloatText(std::to_string(dmg), { e.body.pos.x - 5.0f, e.body.pos.y - e.body.h - 12.0f }, { 1.0f, 0.95f, 0.5f, 1.0f });
}

void Game::onEnemyDeath(const Enemy& e)
{
    kills_++;
    glm::vec2 at = e.body.pos - glm::vec2(0.0f, e.body.h * 0.5f);
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    bool zombie = e.type == Enemy::Type::Zombie;

    int coins = zombie ? 2 + static_cast<int>(u(rng_) * 3.0f) : 1 + static_cast<int>(u(rng_) * 2.0f);
    for (int i = 0; i < coins; ++i) spawnDrop(Drop::Kind::Coin, at);
    if (u(rng_) < (zombie ? 0.35f : 0.18f)) spawnDrop(Drop::Kind::Heart, at);
}

void Game::spawnDrop(Drop::Kind kind, glm::vec2 pos)
{
    std::uniform_real_distribution<float> u(-1.0f, 1.0f);
    Drop d;
    d.kind = kind;
    d.body.pos = pos;
    d.body.hw = 9.0f;
    d.body.h = 20.0f;
    d.body.vel = { u(rng_) * 110.0f, -300.0f - 60.0f * std::fabs(u(rng_)) };
    d.sprite.setup(&tex_.drops, 4, 2, { 4, 4 }, { 8, 5 });
    d.sprite.setAnim(static_cast<int>(kind));
    d.sprite.size = { 24.0f, 24.0f };
    drops_.push_back(d);
}

void Game::breakTile(int tx, int ty)
{
    uint8_t t = world_.get(tx, ty);
    world_.set(tx, ty, AIR);
    inventory_.give(World::drop(t));
    blocksMined_++;

    if (t == TRUNK)   // derrubar o tronco derruba a arvore acima
        for (int y = ty - 1; world_.get(tx, y) == TRUNK; --y)
        {
            world_.set(tx, y, AIR);
            inventory_.give(WOOD);
        }
}

void Game::updateMiningAndPlacing(float dt)
{
    placeCooldown_ = std::max(0.0f, placeCooldown_ - dt);
    attackCooldown_ = std::max(0.0f, attackCooldown_ - dt);
    player_.mining = false;

    glm::vec2 m = input_.mouseWorld;
    int tx = static_cast<int>(std::floor(m.x / TILE)), ty = static_cast<int>(std::floor(m.y / TILE));
    glm::vec2 tileCenter((tx + 0.5f) * TILE, (ty + 0.5f) * TILE);
    bool inReach = glm::length(tileCenter - playerCenter()) <= reach_;

    bool lmb = input_.mouseLeft;
    bool rmb = input_.mouseRight;
    int item = inventory_.selectedItem();

    if (lmb && item == Inventory::TOOL_SWORD)
    {
        // espada inicia um golpe em arco na direcao do mouse (a logica do golpe esta em updateSword)
        player_.startSwing(m.x >= playerCenter().x ? 1 : -1);
        mineProgress_ = 0.0f;
    }
    else if (lmb)
    {
        // 1) bater em inimigo sob o mouse (picareta/blocos causam pouco dano)
        int hit = -1;
        for (size_t i = 0; i < enemies_.size(); ++i)
        {
            AABB big = enemies_[i].body.box();
            big.pMin -= glm::vec2(6.0f); big.pMax += glm::vec2(6.0f);
            if (big.contains(m) && glm::length(enemies_[i].body.pos - playerCenter()) <= reach_ + TILE) hit = static_cast<int>(i);
        }
        if (hit >= 0)
        {
            player_.mining = true;
            player_.facing = m.x >= playerCenter().x ? 1 : -1;
            mineProgress_ = 0.0f;
            if (attackCooldown_ <= 0.0f)
            {
                Enemy& e = enemies_[hit];
                damageEnemy(e, punchDamage_, e.body.pos.x - player_.body.pos.x < 0.0f ? -1 : 1);
                attackCooldown_ = 0.35f;
            }
        }
        // 2) minerar o bloco sob o mouse
        else if (inReach && world_.get(tx, ty) != AIR && world_.get(tx, ty) != BEDROCK)
        {
            if (tx != mineX_ || ty != mineY_) { mineX_ = tx; mineY_ = ty; mineProgress_ = 0.0f; }
            mineProgress_ += dt / World::hardness(world_.get(tx, ty));
            player_.mining = true;
            player_.facing = m.x >= playerCenter().x ? 1 : -1;
            if (mineProgress_ >= 1.0f)
            {
                breakTile(tx, ty);
                mineProgress_ = 0.0f;
                mineX_ = mineY_ = -1;
            }
        }
        else mineProgress_ = 0.0f;
    }
    else mineProgress_ = 0.0f;

    if (!lmb) mineX_ = mineY_ = -1;

    // colocar bloco (botao direito) - so com um bloco selecionado
    if (rmb && item < Inventory::kFirstTool && placeCooldown_ <= 0.0f && inReach && inventory_.selectedCount() > 0 &&
        world_.get(tx, ty) == AIR)
    {
        AABB tile{ {float(tx * TILE), float(ty * TILE)}, {float((tx + 1) * TILE), float((ty + 1) * TILE)} };
        bool blocked = checkCollision(tile, player_.body.box());
        for (const Enemy& e : enemies_) blocked = blocked || checkCollision(tile, e.body.box());
        // regra de vizinhanca precisa encostar em outro bloco
        bool attached = world_.get(tx - 1, ty) != AIR || world_.get(tx + 1, ty) != AIR ||
                        world_.get(tx, ty - 1) != AIR || world_.get(tx, ty + 1) != AIR;
        if (!blocked && attached)
        {
            world_.set(tx, ty, static_cast<uint8_t>(item));
            inventory_.consumeSelected();
            placeCooldown_ = 0.15f;
            player_.facing = m.x >= playerCenter().x ? 1 : -1;
        }
    }
}

// Golpe de espada durante o balanco, uma hitbox AABB na frente do jogador
// verifica colisao com cada inimigo (cada inimigo so leva dano uma vez por golpe).
void Game::updateSword(float dt)
{
    player_.updateSword(dt);
    if (!player_.swordHitboxActive()) return;

    AABB hb = player_.swordHitbox();
    for (Enemy& e : enemies_)
    {
        if (e.hp <= 0) continue;
        if (player_.alreadyHit(e.id)) continue;
        if (checkCollision(hb, e.body.box()))
        {
            player_.registerHit(e.id);
            damageEnemy(e, swordDamage_, player_.swingFacing);
        }
    }
}

void Game::spawnEnemy()
{
    float d = dayNight_.factor(clock_);
    bool night = d < 0.45f;
    int slimes = 0, zombies = 0;
    for (const Enemy& e : enemies_) (e.type == Enemy::Type::Slime ? slimes : zombies)++;
    int slimeLimit = night ? 4 : 2, zombieLimit = night ? 4 : 0;

    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    bool wantZombie = night && zombies < zombieLimit && (u(rng_) < 0.6f || slimes >= slimeLimit);
    if (!wantZombie && slimes >= slimeLimit) return;

    float side = u(rng_) < 0.5f ? -1.0f : 1.0f;
    float x = player_.body.pos.x + side * (450.0f + u(rng_) * 350.0f);
    int tx = std::max(2, std::min(world_.W - 3, static_cast<int>(x / TILE)));
    int ty = world_.topSolid(tx);
    if (world_.get(tx, ty) == TRUNK) return;   // nao nasce em cima de arvore

    glm::vec2 pos(tx * TILE + TILE * 0.5f, static_cast<float>(ty * TILE));
    int dir = side > 0 ? -1 : 1;   // ja nasce virado para o jogador
    if (wantZombie) enemies_.push_back(Enemy::makeZombie(pos, dir, nextEnemyId_++, &tex_.zombie, zombieCfg_, rng_));
    else            enemies_.push_back(Enemy::makeSlime(pos, dir, nextEnemyId_++, &tex_.slime, slimeCfg_, rng_));
}

void Game::updateEnemies(float dt)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);

    // inimigos derrotados (hp <= 0) soltam itens e saem da lista
    for (size_t i = 0; i < enemies_.size();)
    {
        if (enemies_[i].hp <= 0) { onEnemyDeath(enemies_[i]); enemies_.erase(enemies_.begin() + i); }
        else ++i;
    }

    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.0f)
    {
        spawnEnemy();
        spawnTimer_ = dayNight_.factor(clock_) > 0.5f ? 4.0f + u(rng_) * 3.0f : 1.5f + u(rng_) * 2.0f;
    }

    float dayF = dayNight_.factor(clock_);
    EnemyEnv env{ world_, rng_, player_.body.pos, gravity_, zombieSpeed_, clock_ };
    for (size_t i = 0; i < enemies_.size();)
    {
        Enemy& e = enemies_[i];
        float dx = player_.body.pos.x - e.body.pos.x;
        if (e.shouldDespawn(player_.body.pos.x, dayF))
        {
            enemies_.erase(enemies_.begin() + i);
            continue;
        }

        e.update(dt, env);

        // colisao AABB inimigo x jogador
        if (checkCollision(e.body.box(), player_.body.box())) damagePlayer(e.damage, dx >= 0.0f ? 1 : -1, true);
        ++i;
    }
}

// Moedas e coracoes caem com fisica e sao coletados quando a hitbox encosta na do jogador.
void Game::updateDrops(float dt)
{
    AABB pb = player_.body.box();
    pb.pMin -= glm::vec2(8.0f); pb.pMax += glm::vec2(8.0f);

    for (size_t i = 0; i < drops_.size();)
    {
        Drop& d = drops_[i];
        d.update(dt, world_, gravity_);

        bool remove = d.life <= 0.0f;
        if (!remove && d.pickupDelay <= 0.0f && checkCollision(d.body.box(), pb))
        {
            if (d.kind == Drop::Kind::Coin)
            {
                coins_++;
                remove = true;
            }
            else if (player_.hp < player_.maxHp)   // coracao so e pego se estiver machucado
            {
                player_.hp++;
                addFloatText("+1", { player_.body.pos.x - 10.0f, player_.body.pos.y - player_.body.h - 14.0f }, { 0.4f, 1.0f, 0.5f, 1.0f });
                remove = true;
            }
        }
        if (remove) drops_.erase(drops_.begin() + i);
        else ++i;
    }
}
