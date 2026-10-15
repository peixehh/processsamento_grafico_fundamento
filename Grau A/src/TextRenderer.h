#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "Shader.h"

// Texto com FreeType (aula Texturas uma textura por caractere, aula de buffers VBO dinamico).
// Cada glifo vira uma textura GL_RED. Cada caractere e um quad de 6 vertices x 4 floats (x, y, u, v)
// copiado para o mesmo VBO (GL_DYNAMIC_DRAW) com glBufferSubData a cada desenho.
class TextRenderer
{
public:
    // Carrega a fonte em cada tamanho (px) pedido, caracteres Latin-1 (32 a 255). false se algo falhar.
    bool init(const std::string& vertPath, const std::string& fragPath, const std::string& fontPath,
              const std::vector<int>& sizes);

    // Projecao ortografica de tela (0, largura, altura, 0) = origem no canto superior esquerdo, y para baixo.
    void setProjection(float left, float right, float bottom, float top);

    // (x, y) = canto superior esquerdo da linha. Texto em UTF-8, o que nao existe vira '?'.
    void draw(const std::string& utf8, float x, float y, int size, glm::vec4 color);

    float width(const std::string& utf8, int size) const;   // largura em pixels (0 se o tamanho nao foi carregado)
    int lineHeight(int size) const;                         // altura da linha em pixels

private:
    struct Glyph
    {
        GLuint tex = 0;                  // 0 = sem desenho (ex. espaco)
        int w = 0, h = 0;                // tamanho do bitmap
        int bearingX = 0, bearingY = 0;  // deslocamento do bitmap em relacao a linha de base
        int advance = 0;                 // quanto avancar para o proximo caractere
    };
    struct FontSize
    {
        int size = 0;
        int ascent = 0, height = 0;
        Glyph glyphs[224];               // caracteres 32 a 255
    };

    const FontSize* find(int size) const;

    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0;
    std::vector<FontSize> sizes_;
};
