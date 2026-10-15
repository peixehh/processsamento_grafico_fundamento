#include "check.h"
#include "Inventory.h"

int main()
{
    Inventory inv;
    // layout inicial picareta, espada e 6 blocos, tudo zerado, picareta selecionada
    CHECK(inv.item(0) == Inventory::TOOL_PICK);
    CHECK(inv.item(1) == Inventory::TOOL_SWORD);
    CHECK(inv.item(2) == DIRT && inv.item(3) == STONE && inv.item(4) == WOOD);
    CHECK(inv.item(5) == COAL && inv.item(6) == IRON && inv.item(7) == LEAVES);
    for (int i = 0; i < Inventory::kSlots; ++i) CHECK(inv.count(i) == 0);
    CHECK(inv.selected() == 0 && !inv.swordSelected());

    // give soma ate 99, itens que nao estao na barra sao ignorados
    for (int i = 0; i < 120; ++i) inv.give(STONE);
    CHECK(inv.count(3) == 99);
    inv.give(AIR);
    inv.give(GRASS);
    for (int i = 0; i < Inventory::kSlots; ++i)
        if (i != 3) CHECK(inv.count(i) == 0);

    // select e consumeSelected
    inv.select(3);
    CHECK(inv.selected() == 3 && inv.selectedItem() == STONE && inv.selectedCount() == 99);
    inv.consumeSelected();
    CHECK(inv.count(3) == 98);

    // indice invalido e ignorado (nas duas pontas)
    inv.select(-1);
    CHECK(inv.selected() == 3);
    inv.select(Inventory::kSlots);
    CHECK(inv.selected() == 3);
    inv.select(1);
    CHECK(inv.swordSelected());

    // roda do mouse dy > 0 sobe (indice - 1), dy < 0 desce (indice + 1), com volta nas pontas
    inv.select(0);
    inv.scroll(1.0);
    CHECK(inv.selected() == 7);
    inv.scroll(-1.0);
    CHECK(inv.selected() == 0);
    inv.scroll(-1.0);
    CHECK(inv.selected() == 1);
    inv.scroll(0.0);
    CHECK(inv.selected() == 1);    // sem movimento nada muda

    inv.reset();
    CHECK(inv.count(3) == 0 && inv.selected() == 0);
    return resultado();
}
