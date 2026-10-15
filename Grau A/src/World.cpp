#include "World.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
// ---- ruido de valor 2D (usado para as cavernas) ----
float hash2(int x, int y, unsigned seed)
{
    unsigned h = static_cast<unsigned>(x) * 374761393u + static_cast<unsigned>(y) * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / static_cast<float>(0xFFFFFF);
}

float smooth(float t) { return t * t * (3.0f - 2.0f * t); }

float valueNoise(float x, float y, unsigned seed)
{
    int xi = static_cast<int>(std::floor(x)), yi = static_cast<int>(std::floor(y));
    float fx = smooth(x - xi), fy = smooth(y - yi);
    float a = hash2(xi, yi, seed), b = hash2(xi + 1, yi, seed);
    float c = hash2(xi, yi + 1, seed), d = hash2(xi + 1, yi + 1, seed);
    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}

float fbm(float x, float y, unsigned seed)
{
    return 0.65f * valueNoise(x, y, seed) + 0.35f * valueNoise(x * 2.1f, y * 2.1f, seed + 7);
}
}  // namespace

void World::generate(int w, int h, unsigned seed)
{
    W = w; H = h;
    tiles_.assign(static_cast<size_t>(W) * H, AIR);
    heights_.assign(W, 0);
    std::mt19937 rng(seed);
    auto rnd = [&]() { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); };

    // 1) Altura do terreno soma de senoides com fases sorteadas
    float p1 = rnd() * 6.28f, p2 = rnd() * 6.28f, p3 = rnd() * 6.28f;
    int base = H * 3 / 10;
    for (int x = 0; x < W; ++x)
    {
        float v = base + 5.0f * std::sin(x * 0.035f + p1) + 3.0f * std::sin(x * 0.09f + p2) + 1.5f * std::sin(x * 0.21f + p3);
        heights_[x] = static_cast<int>(std::round(v));
    }
    // area de spawn (centro) mais plana
    for (int x = W / 2 - 6; x <= W / 2 + 6; ++x) heights_[x] = heights_[W / 2];

    // 2) Camadas grama, terra, pedra
    for (int x = 0; x < W; ++x)
    {
        int hgt = heights_[x];
        int dirtDepth = 4 + static_cast<int>(rnd() * 3.0f);
        for (int y = hgt; y < H; ++y)
        {
            uint8_t t;
            if (y == hgt) t = GRASS;
            else if (y < hgt + dirtDepth) t = DIRT;
            else t = STONE;
            tiles_[static_cast<size_t>(y) * W + x] = t;
        }
    }

    // 3) Cavernas (tuneis + bolsoes) e minerios, somente abaixo da camada de terra
    for (int x = 1; x < W - 1; ++x)
    {
        for (int y = heights_[x] + 9; y < H - 2; ++y)
        {
            float a = fbm(x * 0.055f, y * 0.075f, seed);
            float b = fbm(x * 0.045f + 91.0f, y * 0.06f + 37.0f, seed + 3);
            bool tunnel = std::fabs(a - 0.5f) < 0.032f;
            bool pocket = b > 0.67f;
            uint8_t& t = tiles_[static_cast<size_t>(y) * W + x];
            if (tunnel || pocket)
            {
                t = AIR;
                continue;
            }
            int depth = y - heights_[x];
            if (t == STONE)
            {
                float r = rnd();
                if (depth > 8 && r < 0.028f) t = COAL;
                else if (depth > 24 && r > 0.984f) t = IRON;
            }
        }
    }

    // 4) Arvores sobre a grama
    for (int x = 6; x < W - 6; x += 5 + static_cast<int>(rnd() * 6.0f))
    {
        int g = heights_[x];
        if (std::abs(heights_[x - 2] - g) > 1 || std::abs(heights_[x + 2] - g) > 1) continue;
        if (std::abs(x - W / 2) < 3) continue;  // nao nasce em cima do jogador
        int trunk = 4 + static_cast<int>(rnd() * 3.0f);
        for (int i = 1; i <= trunk; ++i) set(x, g - i, TRUNK);
        int top = g - trunk;
        for (int dy = -2; dy <= 1; ++dy)
            for (int dx = -2; dx <= 2; ++dx)
            {
                float e = (dx * dx) / 7.0f + (dy * dy) / 5.0f;
                if (e <= 1.0f && get(x + dx, top + dy) == AIR) set(x + dx, top + dy, LEAVES);
            }
    }

    // 5) Bedrock chao e paredes laterais (indestrutiveis)
    for (int x = 0; x < W; ++x) { set(x, H - 1, BEDROCK); }
    for (int y = 0; y < H; ++y) { set(0, y, BEDROCK); set(W - 1, y, BEDROCK); }
}

uint8_t World::get(int x, int y) const
{
    if (y < 0) return AIR;
    if (x < 0 || x >= W || y >= H) return BEDROCK;
    return tiles_[static_cast<size_t>(y) * W + x];
}

void World::set(int x, int y, uint8_t t)
{
    if (x < 0 || x >= W || y < 0 || y >= H) return;
    tiles_[static_cast<size_t>(y) * W + x] = t;
}

int World::surface(int x) const
{
    x = std::max(0, std::min(W - 1, x));
    return heights_[x];
}

int World::topSolid(int x) const
{
    for (int y = 0; y < H; ++y)
        if (solid(x, y)) return y;
    return H - 1;
}

bool World::underground(int x, int y) const
{
    return y > surface(x) + 1;
}

bool World::collides(const AABB& box) const
{
    int x0 = static_cast<int>(std::floor(box.pMin.x / TILE)), x1 = static_cast<int>(std::floor(box.pMax.x / TILE));
    int y0 = static_cast<int>(std::floor(box.pMin.y / TILE)), y1 = static_cast<int>(std::floor(box.pMax.y / TILE));
    for (int ty = y0; ty <= y1; ++ty)
        for (int tx = x0; tx <= x1; ++tx)
        {
            if (!solid(tx, ty)) continue;
            AABB t{ {float(tx * TILE), float(ty * TILE)}, {float((tx + 1) * TILE), float((ty + 1) * TILE)} };
            if (checkCollision(box, t)) return true;
        }
    return false;
}

float World::hardness(uint8_t t)
{
    switch (t)
    {
    case LEAVES: return 0.20f;
    case GRASS:
    case DIRT:   return 0.35f;
    case WOOD:
    case TRUNK:  return 0.60f;
    case STONE:  return 0.90f;
    case COAL:   return 1.10f;
    case IRON:   return 1.40f;
    default:     return 1e9f;   // ar / bedrock nao quebra
    }
}

uint8_t World::drop(uint8_t t)
{
    if (t == GRASS) return DIRT;
    if (t == TRUNK) return WOOD;
    if (t == AIR || t == BEDROCK) return AIR;
    return t;
}
