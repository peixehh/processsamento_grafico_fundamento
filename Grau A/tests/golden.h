// Gerado por golden_capture.cpp a partir do codigo ORIGINAL (antes da refatoracao). Nao editar a mao.
#pragma once
#include <cstdint>

struct GoldenMundo { unsigned seed; uint64_t hash; int surface10, surface150, topSolid150; };
struct GoldenCorpo { float x, y, vx, vy; bool noChao; float minY; };

static const GoldenMundo kGoldenMundos[3] = {
    { 1234u, 0x27c77e2ef718dfa2ull, 32, 27, 27 },
    { 1u, 0xea850be15c514b69ull, 34, 36, 36 },
    { 42u, 0xe4a47402b333645cull, 32, 33, 33 },
};

static const GoldenCorpo kGoldenQueda = { 176.000000f, 640.000000f, 0.000000f, 0.000000f, true, 300.000000f };
static const GoldenCorpo kGoldenParede = { 949.000000f, 640.000000f, 0.000000f, 0.000000f, true, 640.000000f };
static const GoldenCorpo kGoldenTeto = { 368.000000f, 640.000000f, 0.000000f, 0.000000f, true, 542.000000f };
static const GoldenCorpo kGoldenDegrau = { 469.000000f, 640.000000f, 0.000000f, 0.000000f, true, 640.000000f };

static const bool kGoldenAABB[5] = { true, false, false, true, false };
static const bool kGoldenContem[4] = { true, true, false, false };
static const int kGoldenFrames[30] = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 0 };
