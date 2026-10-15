#include "TextRenderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cmath>
#include <cstring>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
const unsigned kSemCodigo = 0xFFFF;   // "nao existe na fonte"

// Le o proximo caractere de uma string UTF-8 e avanca i. Sequencias invalidas ou truncadas
// consomem so o que existe e devolvem kSemCodigo (vira '?').
unsigned proximoCodigo(const std::string& s, size_t& i)
{
    unsigned char c = static_cast<unsigned char>(s[i++]);
    if (c < 0x80) return c;

    int extras = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;   // bytes de continuacao esperados
    unsigned codigo = extras == 1 ? (c & 0x1Fu) : kSemCodigo;
    int lidos = 0;
    while (lidos < extras && i < s.size() && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80)
    {
        if (extras == 1) codigo = (codigo << 6) | (static_cast<unsigned char>(s[i]) & 0x3Fu);
        ++i;
        ++lidos;
    }
    return lidos < extras ? kSemCodigo : codigo;
}

// Indice no vetor de glifos (32 a 255), o resto vira '?'
int indiceDoGlifo(unsigned codigo)
{
    if (codigo < 32 || codigo > 255) codigo = '?';
    return static_cast<int>(codigo) - 32;
}
}  // namespace

bool TextRenderer::init(const std::string& vertPath, const std::string& fragPath, const std::string& fontPath,
                        const std::vector<int>& sizes)
{
    if (!shader_.load(vertPath, fragPath)) return false;

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "FreeType: nao consegui iniciar a biblioteca\n";
        return false;
    }
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "FreeType: nao consegui abrir a fonte " << fontPath << "\n";
        FT_Done_FreeType(ft);
        return false;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // bitmaps de 1 byte por pixel (aula Texturas)
    for (int size : sizes)
    {
        FT_Set_Pixel_Sizes(face, 0, size);
        FontSize fs;
        fs.size = size;
        fs.ascent = static_cast<int>(face->size->metrics.ascender >> 6);
        fs.height = static_cast<int>((face->size->metrics.ascender - face->size->metrics.descender) >> 6);

        for (unsigned c = 32; c < 256; ++c)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;   // sem desenho fica zerado
            Glyph& g = fs.glyphs[c - 32];
            const FT_Bitmap& bm = face->glyph->bitmap;
            g.w = static_cast<int>(bm.width);
            g.h = static_cast<int>(bm.rows);
            g.bearingX = face->glyph->bitmap_left;
            g.bearingY = face->glyph->bitmap_top;
            g.advance = static_cast<int>(face->glyph->advance.x >> 6);
            if (g.w == 0 || g.h == 0) continue;                    // espaco so avanca

            // copia linha a linha para um bloco sem sobra (o pitch do FreeType pode ser maior que a largura)
            std::vector<unsigned char> pixels(static_cast<size_t>(g.w) * g.h);
            for (int row = 0; row < g.h; ++row)
                std::memcpy(&pixels[static_cast<size_t>(row) * g.w], bm.buffer + row * bm.pitch, static_cast<size_t>(g.w));

            glGenTextures(1, &g.tex);
            glBindTexture(GL_TEXTURE_2D, g.tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g.w, g.h, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        sizes_.push_back(fs);
    }
    FT_Done_Face(face);          // ja temos todos os glifos em texturas
    FT_Done_FreeType(ft);

    // VAO + VBO dinamico 6 vertices x 4 floats (x, y, u, v), reescrito a cada caractere
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shader_.use();
    shader_.setInt("tex_buffer", 0);
    return true;
}

void TextRenderer::setProjection(float left, float right, float bottom, float top)
{
    shader_.use();
    shader_.setMat4("projection", glm::ortho(left, right, bottom, top));
}

void TextRenderer::draw(const std::string& utf8, float x, float y, int size, glm::vec4 color)
{
    const FontSize* fs = find(size);
    if (!fs) return;

    shader_.use();
    shader_.setVec4("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao_);

    float baseline = std::floor(y) + fs->ascent;
    x = std::floor(x);                       // posicoes inteiras mantem o texto nitido
    for (size_t i = 0; i < utf8.size();)
    {
        const Glyph& g = fs->glyphs[indiceDoGlifo(proximoCodigo(utf8, i))];
        if (g.tex != 0)
        {
            float xpos = x + g.bearingX;
            float ypos = baseline - g.bearingY;
            float w = static_cast<float>(g.w), h = static_cast<float>(g.h);
            float vertices[6][4] = {
                { xpos,     ypos,     0.0f, 0.0f },
                { xpos,     ypos + h, 0.0f, 1.0f },
                { xpos + w, ypos + h, 1.0f, 1.0f },

                { xpos,     ypos,     0.0f, 0.0f },
                { xpos + w, ypos + h, 1.0f, 1.0f },
                { xpos + w, ypos,     1.0f, 0.0f }
            };
            glBindTexture(GL_TEXTURE_2D, g.tex);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        x += static_cast<float>(g.advance);
    }
}

float TextRenderer::width(const std::string& utf8, int size) const
{
    const FontSize* fs = find(size);
    if (!fs) return 0.0f;
    float total = 0.0f;
    for (size_t i = 0; i < utf8.size();)
        total += static_cast<float>(fs->glyphs[indiceDoGlifo(proximoCodigo(utf8, i))].advance);
    return total;
}

int TextRenderer::lineHeight(int size) const
{
    const FontSize* fs = find(size);
    return fs ? fs->height : 0;
}

const TextRenderer::FontSize* TextRenderer::find(int size) const
{
    for (const FontSize& fs : sizes_)
        if (fs.size == size) return &fs;
    return nullptr;
}
