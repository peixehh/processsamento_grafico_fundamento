#include "check.h"
#include "arena.h"
#include "Player.h"

static Player novoJogador(float x, float y)
{
    Player p;
    p.body = novoCorpo(x, y);
    p.sprite.setup(nullptr, 6, 4, { 4, 6, 2, 4 }, { 4, 12, 0, 10 });
    p.apexY = y;
    return p;
}

int main()
{
    const float dt = 1.0f / 60.0f;
    World w;
    montarArena(w);
    PlayerParams pp;   // valores padrao = os do config.txt
    Input parado, direita, esquerda, pula;
    direita.moveX = 1.0f;
    esquerda.moveX = -1.0f;
    pula.jump = true;

    // ---- andar acelera suavemente ate a velocidade-alvo e vira para o lado certo ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        for (int i = 0; i < 5; ++i) p.update(dt, parado, w, pp);
        CHECK(p.body.onGround);
        CHECK(std::fabs(p.body.vel.x) < 1e-3);
        for (int i = 0; i < 60; ++i) p.update(dt, direita, w, pp);
        CHECK(p.body.vel.x > 200.0f && p.body.vel.x <= 240.0f);
        CHECK(p.facing == 1);
        for (int i = 0; i < 60; ++i) p.update(dt, esquerda, w, pp);
        CHECK(p.facing == -1);
    }

    // ---- pular so no chao, segurar o botao no ar nao pula de novo ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        for (int i = 0; i < 5; ++i) p.update(dt, parado, w, pp);
        p.update(dt, pula, w, pp);
        CHECK(p.body.vel.y < 0.0f && !p.body.onGround);
        float vy = p.body.vel.y;
        p.update(dt, pula, w, pp);
        CHECK(p.body.vel.y > vy);                  // no ar so a gravidade age, o pulo nao e aplicado de novo
    }

    // ---- logo apos o dano (invulnerabilidade >= 1 s) o controle fica travado ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        for (int i = 0; i < 5; ++i) p.update(dt, parado, w, pp);
        p.invuln = 1.5f;
        p.update(dt, direita, w, pp);
        CHECK(std::fabs(p.body.vel.x) < 1e-6);
        CHECK(p.facing == 1);
    }

    // ---- dano de queda 192 px exatos = sem dano, 193 px = 1 coracao, vida nunca abaixo de 0 ----
    struct Caso { float queda; int dano; };
    const Caso casos[] = {
        { 192.0f, 0 },     // no limite nao e dano
        { 193.0f, 1 },
        { 287.0f, 1 },
        { 288.0f, 2 },     // 1 + (96 / 96)
        { 400.0f, 3 },     // 1 + int(208 / 96)
        { 100.0f, 0 },
        { 48.0f, 0 },
        { 10.0f, 0 },
    };
    for (const Caso& c : casos)
    {
        Player p = novoJogador(5 * 32 + 16, 639.0f);   // a 1 px do chao, caindo
        p.wasGround = false;
        p.apexY = 640.0f - c.queda;
        p.body.vel.y = 200.0f;
        PlayerEvents ev = p.update(dt, parado, w, pp);
        CHECK(p.body.onGround);
        CHECK(ev.fallDamage == c.dano);
        CHECK(p.hp == 5 - c.dano);
    }

    // ---- takeDamage vida minima 0, recuo, invulnerabilidade bloqueia o segundo golpe de inimigo ----
    {
        Player p;
        p.hp = 2;
        CHECK(p.takeDamage(5, 0, false));
        CHECK(p.hp == 0);

        Player q;
        CHECK(q.takeDamage(1, 1, true));
        CHECK(q.hp == 4);
        CHECK_NEAR(q.body.vel.x, 300.0, 1e-6);
        CHECK_NEAR(q.body.vel.y, -330.0, 1e-6);
        CHECK_NEAR(q.invuln, 1.2, 1e-6);
        CHECK(!q.takeDamage(1, 1, true));      // golpe de inimigo durante a invulnerabilidade ignorado
        CHECK(q.hp == 4);
        CHECK(q.takeDamage(1, 0, false));      // dano de queda nao respeita a invulnerabilidade
        CHECK(q.hp == 3);
        CHECK_NEAR(q.invuln, 1.2, 1e-6);       // fica o maior valor
    }

    // ---- regeneracao 1 coracao a cada 12 s sem dano ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        p.hp = 3;
        for (int i = 0; i < 700; ++i) p.update(dt, parado, w, pp);    // 11,7 s
        CHECK(p.hp == 3);
        for (int i = 0; i < 30; ++i) p.update(dt, parado, w, pp);     // 12,2 s
        CHECK(p.hp == 4);
    }

    // ---- espada linha do tempo do golpe ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        CHECK(!p.swordHitboxActive());
        CHECK(p.startSwing(1));
        CHECK(!p.startSwing(-1));                  // ja esta no meio de um golpe
        CHECK(p.swinging && p.swingFacing == 1);
        p.updateSword(0.05f);                      // 18% do golpe ainda nao acerta
        CHECK(!p.swordHitboxActive());
        p.updateSword(0.02f);                      // 25% janela de acerto aberta (20% a 90%)
        CHECK(p.swordHitboxActive());
        AABB hb = p.swordHitbox();
        glm::vec2 c = p.center();
        CHECK_NEAR(hb.pMin.x, c.x, 1e-4);
        CHECK_NEAR(hb.pMax.x, c.x + 76.0, 1e-4);
        CHECK_NEAR(hb.pMin.y, c.y - 56.0, 1e-4);
        CHECK_NEAR(hb.pMax.y, c.y + 30.0, 1e-4);
        CHECK(!p.alreadyHit(7));                   // cada inimigo so leva dano uma vez por golpe
        p.registerHit(7);
        CHECK(p.alreadyHit(7) && !p.alreadyHit(8));
        p.updateSword(0.19f);                      // 93% janela fechou
        CHECK(!p.swordHitboxActive());
        p.updateSword(0.05f);                      // passou de 0,28 s termina e entra em recarga
        CHECK(!p.swinging);
        CHECK_NEAR(p.swingCooldown, 0.12, 1e-6);
        CHECK(!p.startSwing(1));                   // recarga
        p.updateSword(0.13f);
        CHECK(p.startSwing(-1));                   // recarga acabou, novo golpe para a esquerda
        CHECK(p.swingHits.empty());                // a lista de acertos recomeca
        p.updateSword(0.07f);
        CHECK(p.facing == -1);
        AABB esq = p.swordHitbox();
        glm::vec2 c2 = p.center();
        CHECK_NEAR(esq.pMin.x, c2.x - 76.0, 1e-4);
        CHECK_NEAR(esq.pMax.x, c2.x, 1e-4);
    }

    // ---- animacao parado 0, andando 1, no ar 2 (quadro 0 subindo, 1 descendo), minerando 3 ----
    {
        Player p = novoJogador(5 * 32 + 16, 640.0f);
        p.body.onGround = true;
        p.body.vel.x = 0.0f;
        p.animate(dt);
        CHECK(p.sprite.iAnimation == 0);
        p.body.vel.x = 100.0f;
        p.animate(dt);
        CHECK(p.sprite.iAnimation == 1);
        p.body.onGround = false;
        p.body.vel.y = -100.0f;
        p.animate(dt);
        CHECK(p.sprite.iAnimation == 2 && p.sprite.iFrame == 0);
        p.body.vel.y = 100.0f;
        p.animate(dt);
        CHECK(p.sprite.iFrame == 1);
        p.mining = true;
        p.animate(dt);
        CHECK(p.sprite.iAnimation == 3);
        p.facing = -1;
        p.animate(dt);
        CHECK(p.sprite.flipX);
    }
    return resultado();
}
