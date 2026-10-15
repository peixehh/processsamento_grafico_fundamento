#include "Hud.h"

#include <algorithm>
#include <cmath>

namespace
{
const char* itemName(int t)
{
    switch (t)
    {
    case Inventory::TOOL_PICK:  return "Picareta";
    case Inventory::TOOL_SWORD: return "Espada";
    case DIRT:   return "Terra";
    case STONE:  return "Pedra";
    case WOOD:   return "Madeira";
    case COAL:   return "Carvão";
    case IRON:   return "Ferro";
    case LEAVES: return "Folhas";
    default:     return "";
    }
}
}  // namespace

void Hud::init(SpriteRenderer* renderer, TextRenderer* text, const Texturas* tex,
               int sizeSmall, int sizeMedium, int sizeBig, float viewW, float viewH)
{
    renderer_ = renderer;
    text_ = text;
    tex_ = tex;
    sizeSmall_ = sizeSmall;
    sizeMedium_ = sizeMedium;
    sizeBig_ = sizeBig;
    viewW_ = viewW;
    viewH_ = viewH;
}

void Hud::beginScreen()
{
    text_->setProjection(0.0f, viewW_, viewH_, 0.0f);
}

// Sombra preta deslocada (1 px a cada 16 px de fonte) e depois o texto, como no atlas antigo.
void Hud::drawText(const std::string& s, float x, float y, int size, glm::vec4 color)
{
    float sombra = static_cast<float>(size) / 16.0f;
    text_->draw(s, x + sombra, y + sombra, size, { 0.0f, 0.0f, 0.0f, 0.65f * color.a });
    text_->draw(s, x, y, size, color);
}

void Hud::drawTextCentered(const std::string& s, float y, int size, glm::vec4 color)
{
    drawText(s, (viewW_ - text_->width(s, size)) * 0.5f, y, size, color);
}

void Hud::drawTextRight(const std::string& s, float rightX, float y, int size, glm::vec4 color)
{
    drawText(s, rightX - text_->width(s, size), y, size, color);
}

void Hud::drawTitle(float clock)
{
    renderer_->draw(tex_->white, { 0, 0 }, { viewW_, viewH_ }, { 0, 0 }, { 1, 1 }, { 0.0f, 0.0f, 0.0f, 0.45f });
    drawTextCentered("TERRARIA :D", 100.0f, sizeBig_, { 1.0f, 0.9f, 0.45f, 1.0f });
    if (std::fmod(clock, 1.2f) < 0.8f) drawTextCentered("Pressione ENTER para jogar", 245.0f, sizeMedium_);
    drawTextCentered("A/D ou setas: andar    W/Espaço: pular", 330.0f, sizeSmall_);
    drawTextCentered("Botão esquerdo: minerar / golpear com a espada    Botão direito: colocar bloco", 355.0f, sizeSmall_);
    drawTextCentered("1-8 ou roda do mouse: escolher item (2 = espada)    ESC: sair", 380.0f, sizeSmall_);
}

void Hud::drawPlaying(const HudInfo& info)
{
    const Inventory& inv = *info.inventory;

    // barra de itens no canto superior ESQUERDO (slots.png quadro 0 normal, quadro 1 selecionado)
    const float slotSize = 40.0f, gap = 6.0f;
    const float sx = 14.0f, sy = 12.0f;
    const float tdx = 1.0f / ATLAS_TILES;
    for (int i = 0; i < Inventory::kSlots; ++i)
    {
        float x = sx + i * (slotSize + gap);
        renderer_->draw(tex_->slots, { x, sy }, { slotSize, slotSize }, { i == inv.selected() ? 0.5f : 0.0f, 0.0f }, { 0.5f, 1.0f });
        int item = inv.item(i);
        if (item >= Inventory::kFirstTool)   // ferramenta desenho em items.png (quadro 0 = picareta, 1 = espada)
            renderer_->draw(tex_->items, { x + 4.0f, sy + 4.0f }, { 32.0f, 32.0f },
                            { (item - Inventory::kFirstTool) * 0.5f, 0.0f }, { 0.5f, 1.0f });
        else
        {
            float dim = inv.count(i) > 0 ? 1.0f : 0.35f;
            renderer_->draw(tex_->tiles, { x + 4.0f, sy + 4.0f }, { 32.0f, 32.0f },
                            { atlasIndex(static_cast<uint8_t>(item)) * tdx, 0.0f }, { tdx, 1.0f }, { dim, dim, dim, 1.0f });
            if (inv.count(i) > 0)
            {
                std::string n = std::to_string(inv.count(i));
                drawText(n, x + slotSize - 4.0f - text_->width(n, sizeSmall_),
                         sy + slotSize - text_->lineHeight(sizeSmall_) - 2.0f, sizeSmall_);
            }
        }
        drawText(std::to_string(i + 1), x + 4.0f, sy + 2.0f, sizeSmall_, { 1.0f, 1.0f, 1.0f, 0.55f });   // numero da tecla
    }
    std::string name = itemName(inv.selectedItem());
    renderer_->draw(tex_->white, { sx, sy + slotSize + 4.0f }, { text_->width(name, sizeSmall_) + 10.0f, 24.0f },
                    { 0, 0 }, { 1, 1 }, { 0.0f, 0.0f, 0.0f, 0.4f });
    drawText(name, sx + 5.0f, sy + slotSize + 7.0f, sizeSmall_, { 1.0f, 0.95f, 0.7f, 1.0f });

    // coracoes no canto superior DIREITO
    for (int i = 0; i < info.maxHp; ++i)
    {
        int f = i < info.hp ? 0 : 1;
        float x = viewW_ - 14.0f - (info.maxHp - i) * 34.0f + 2.0f;
        renderer_->draw(tex_->hearts, { x, 12.0f }, { 32.0f, 32.0f }, { f * 0.5f, 0.0f }, { 0.5f, 1.0f });
    }

    // moedas, profundidade e hora logo abaixo dos coracoes
    float rx = viewW_ - 14.0f;
    renderer_->draw(tex_->white, { rx - 118.0f, 48.0f }, { 126.0f, 76.0f }, { 0, 0 }, { 1, 1 }, { 0.0f, 0.0f, 0.0f, 0.4f });   // fundo para dar contraste
    renderer_->draw(tex_->drops, { rx - 24.0f, 50.0f }, { 24.0f, 24.0f }, { 0.0f, 0.0f }, { 0.25f, 0.5f });
    drawTextRight(std::to_string(info.coins), rx - 30.0f, 52.0f, sizeSmall_, { 1.0f, 0.9f, 0.4f, 1.0f });
    drawTextRight("Prof: " + std::to_string(info.depth), rx, 78.0f, sizeSmall_);
    drawTextRight(info.isDay ? "Dia" : "Noite", rx, 100.0f, sizeSmall_);
}

void Hud::drawGameOver(const HudInfo& info, float clock)
{
    renderer_->draw(tex_->white, { 0, 0 }, { viewW_, viewH_ }, { 0, 0 }, { 1, 1 }, { 0.45f, 0.0f, 0.0f, 0.55f });
    drawTextCentered("VOCÊ MORREU", 120.0f, sizeBig_, { 1.0f, 0.4f, 0.4f, 1.0f });
    drawTextCentered("Tempo: " + std::to_string(static_cast<int>(info.survived)) + "s   Blocos: " + std::to_string(info.blocksMined) +
                     "   Inimigos: " + std::to_string(info.kills) + "   Moedas: " + std::to_string(info.coins), 260.0f, sizeMedium_);
    if (std::fmod(clock, 1.2f) < 0.8f) drawTextCentered("ENTER para recomeçar", 340.0f, sizeMedium_);
}

void Hud::drawFloatTexts(const std::vector<FloatText>& texts, const Camera& camera)
{
    text_->setProjection(camera.left(), camera.right(), camera.bottom(), camera.top());
    for (const FloatText& f : texts)
    {
        glm::vec4 col = f.color;
        col.a = std::min(1.0f, f.life / (f.maxLife * 0.5f));
        drawText(f.text, f.pos.x, f.pos.y, sizeSmall_, col);
    }
}
