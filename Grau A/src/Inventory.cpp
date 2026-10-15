#include "Inventory.h"

#include <algorithm>

void Inventory::reset()
{
    const int itens[kSlots] = { TOOL_PICK, TOOL_SWORD, DIRT, STONE, WOOD, COAL, IRON, LEAVES };
    for (int i = 0; i < kSlots; ++i)
    {
        item_[i] = itens[i];
        count_[i] = 0;
    }
    selected_ = 0;
}

void Inventory::select(int slot)
{
    if (slot >= 0 && slot < kSlots) selected_ = slot;
}

void Inventory::scroll(double dy)
{
    if (dy == 0.0) return;
    selected_ = (selected_ + (dy > 0 ? -1 : 1) + kSlots) % kSlots;
}

void Inventory::give(int itemId)
{
    for (int i = 0; i < kSlots; ++i)
        if (item_[i] == itemId) count_[i] = std::min(99, count_[i] + 1);
}
