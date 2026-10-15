#pragma once
#include "Texture.h"

// Todas as texturas do jogo num lugar so (o Hud recebe uma referencia constante a esta struct).
struct Texturas
{
    Texture tiles, crack, hearts, slots, items, drops, white;   // mundo e HUD
    Texture player, slime, zombie;                              // spritesheets
};
