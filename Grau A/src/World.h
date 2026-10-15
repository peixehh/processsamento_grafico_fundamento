#pragma once
#include <cstdint>
#include <vector>

#include "AABB.h"

constexpr int TILE = 32;   // tamanho de um bloco em unidades de mundo (arte 16px x 2)

// Os valores sao tambem o indice do bloco no atlas assets/tiles.png
// WOOD..BEDROCK sao as 9 celulas do atlas. TRUNK e o tronco natural das arvores
// usa o desenho da madeira, mas e atravessavel (fica "no fundo", como no Terraria).
enum Tile : uint8_t { AIR = 0, GRASS, DIRT, STONE, COAL, IRON, WOOD, LEAVES, BEDROCK, TRUNK };
constexpr int ATLAS_TILES = 9;   // numero de celulas em assets/tiles.png
inline int atlasIndex(uint8_t t) { return t == TRUNK ? static_cast<int>(WOOD) : static_cast<int>(t); }

class World
{
public:
    int W = 0, H = 0;

    void generate(int w, int h, unsigned seed);

    uint8_t get(int x, int y) const;      // fora do mapa laterais/fundo = bedrock, acima = ar
    void set(int x, int y, uint8_t t);
    bool solid(int x, int y) const { uint8_t t = get(x, y); return t != AIR && t != LEAVES && t != TRUNK; }

    int surface(int x) const;             // altura (em blocos) do terreno na coluna x
    int topSolid(int x) const;            // primeiro bloco solido de cima para baixo
    bool underground(int x, int y) const; // abaixo da superficie recebe a parede escura da caverna

    // Existe algum bloco solido sobreposto a esta caixa?
    bool collides(const AABB& box) const;

    static float hardness(uint8_t t);     // segundos para quebrar
    static uint8_t drop(uint8_t t);       // item obtido ao quebrar (AIR = nada)

private:
    std::vector<uint8_t> tiles_;
    std::vector<int> heights_;
};
