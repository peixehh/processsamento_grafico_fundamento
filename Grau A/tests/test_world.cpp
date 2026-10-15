#include "check.h"
#include "golden.h"
#include "World.h"

static uint64_t hashMundo(const World& w)
{
    uint64_t h = 1469598103934665603ull;   // FNV-1a (a mesma do golden_capture)
    for (int y = 0; y < w.H; ++y)
        for (int x = 0; x < w.W; ++x)
        {
            h ^= w.get(x, y);
            h *= 1099511628211ull;
        }
    return h;
}

int main()
{
    // 1) mesma semente => exatamente o mesmo mundo do codigo original
    for (const GoldenMundo& g : kGoldenMundos)
    {
        World w;
        w.generate(300, 100, g.seed);
        CHECK(hashMundo(w) == g.hash);
        CHECK(w.surface(10) == g.surface10);
        CHECK(w.surface(150) == g.surface150);
        CHECK(w.topSolid(150) == g.topSolid150);
    }

    // 2) bordas bedrock nas laterais e no fundo, ar acima do mapa
    World w;
    w.generate(300, 100, 1234);
    CHECK(w.get(-1, 5) == BEDROCK);
    CHECK(w.get(0, 5) == BEDROCK);
    CHECK(w.get(299, 5) == BEDROCK);
    CHECK(w.get(300, 5) == BEDROCK);
    CHECK(w.get(5, 99) == BEDROCK);
    CHECK(w.get(5, 100) == BEDROCK);
    CHECK(w.get(5, -1) == AIR);

    // 3) set fora do mapa e ignorado, folhas e tronco sao atravessaveis
    w.set(-1, 0, STONE);
    CHECK(w.get(-1, 0) == BEDROCK);
    w.set(5, 5, STONE);
    CHECK(w.solid(5, 5));
    w.set(5, 5, LEAVES);
    CHECK(!w.solid(5, 5));
    w.set(5, 5, TRUNK);
    CHECK(!w.solid(5, 5));
    w.set(5, 5, AIR);
    CHECK(!w.solid(5, 5));

    // 4) dureza e itens obtidos
    CHECK_NEAR(World::hardness(LEAVES), 0.20, 1e-6);
    CHECK_NEAR(World::hardness(DIRT), 0.35, 1e-6);
    CHECK_NEAR(World::hardness(STONE), 0.90, 1e-6);
    CHECK_NEAR(World::hardness(IRON), 1.40, 1e-6);
    CHECK(World::hardness(BEDROCK) > 1e8);
    CHECK(World::drop(GRASS) == DIRT);
    CHECK(World::drop(TRUNK) == WOOD);
    CHECK(World::drop(STONE) == STONE);
    CHECK(World::drop(AIR) == AIR);
    CHECK(World::drop(BEDROCK) == AIR);
    return resultado();
}
