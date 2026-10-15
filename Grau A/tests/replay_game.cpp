// Versao refatorada de Game::runReplay o corpo e o mesmo da copia do codigo original (replay_body.inc).
#include "Game.h"
#include "replay_common.h"

#define REPLAY_SELECT(i) inventory_.select(i)
#define REPLAY_GIVE_BLOCKS() \
    for (int i_ = 2; i_ < Inventory::kSlots; ++i_) \
        for (int n_ = 0; n_ < 30; ++n_) inventory_.give(inventory_.item(i_))
#define REPLAY_PROCESS_INPUT() processInput()

#include "replay_body.inc"
