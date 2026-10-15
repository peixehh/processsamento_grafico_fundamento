#pragma once
#include "World.h"

// Barra de itens 8 espacos (picareta, espada e 6 tipos de bloco) com contador.
class Inventory
{
public:
    static constexpr int kSlots = 8;
    static constexpr int kFirstTool = 100;   // ids >= 100 sao ferramentas (os blocos usam o valor do enum Tile)
    enum Tool { TOOL_PICK = kFirstTool, TOOL_SWORD = kFirstTool + 1 };

    Inventory() { reset(); }

    void reset();                              // zera os contadores e seleciona o primeiro espaco
    int item(int slot) const { return item_[slot]; }
    int count(int slot) const { return count_[slot]; }
    int selected() const { return selected_; }
    int selectedItem() const { return item_[selected_]; }
    int selectedCount() const { return count_[selected_]; }
    bool swordSelected() const { return selectedItem() == TOOL_SWORD; }

    void select(int slot);                     // teclas 1 a 8, indice fora da faixa e ignorado
    void scroll(double dy);                    // roda do mouse dy > 0 sobe, dy < 0 desce, com volta nas pontas
    void give(int itemId);                     // +1 no espaco desse item (maximo 99)
    void consumeSelected() { --count_[selected_]; }

private:
    int item_[kSlots];
    int count_[kSlots];
    int selected_;
};
