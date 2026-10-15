#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"

// Desenha qualquer objeto 2D guarda o quad unitario (VAO + VBO + EBO) UMA vez
// e reutiliza para todos os sprites cada objeto so muda a matriz de modelo
class SpriteRenderer
{
public:
    bool init(const std::string& vertPath, const std::string& fragPath)
    {
        if (!shader_.load(vertPath, fragPath)) return false;

        // x, y, u, v  (quad de 0 a 1, a escala da matriz de modelo define o tamanho)
        float vertices[] = {
            0.0f, 0.0f, 0.0f, 0.0f,   // top esquerda
            1.0f, 0.0f, 1.0f, 0.0f,   // top direita
            1.0f, 1.0f, 1.0f, 1.0f,   // base direita
            0.0f, 1.0f, 0.0f, 1.0f    // base esquerda
        };
        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        shader_.use();
        shader_.setInt("tex_buffer", 0);
        return true;
    }

    // Matriz de projecao ortografica origem no canto superior esquerdo, y para baixo.
    void setProjection(float left, float right, float bottom, float top)
    {
        shader_.use();
        shader_.setMat4("projection", glm::ortho(left, right, bottom, top));
    }

    // pos = canto superior esquerdo, size = largura/altura em unidades de mundo.
    // uvOff/uvScale selecionam a celula do spritesheet (ou a janela da camada de parallax).
    void draw(const Texture& tex, glm::vec2 pos, glm::vec2 size,
              glm::vec2 uvOff = {0.0f, 0.0f}, glm::vec2 uvScale = {1.0f, 1.0f},
              glm::vec4 tint = {1.0f, 1.0f, 1.0f, 1.0f}, bool flipX = false)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(pos, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        if (flipX)  // espelha DENTRO da celula comeca na borda direita e anda para a esquerda
        {
            uvOff.x += uvScale.x;
            uvScale.x = -uvScale.x;
        }

        shader_.use();
        shader_.setMat4("model", model);
        shader_.setVec2("uvOffset", uvOff);
        shader_.setVec2("uvScale", uvScale);
        shader_.setVec4("tint", tint);
        tex.bind(0);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Igual a draw(), mas o quad e rotacionado em torno de um ponto de apoio ("pivot", em 0..1
    // dentro do quad). Matriz de modelo translate(pivot no mundo) * rotate * translate(-pivot) * scale.
    void drawRotated(const Texture& tex, glm::vec2 pivotWorld, glm::vec2 size, glm::vec2 pivotNorm, float angleRad,
                     glm::vec2 uvOff, glm::vec2 uvScale, glm::vec4 tint = {1.0f, 1.0f, 1.0f, 1.0f}, bool flipX = false)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(pivotWorld, 0.0f));
        model = glm::rotate(model, angleRad, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-pivotNorm * size, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        if (flipX) { uvOff.x += uvScale.x; uvScale.x = -uvScale.x; }

        shader_.use();
        shader_.setMat4("model", model);
        shader_.setVec2("uvOffset", uvOff);
        shader_.setVec2("uvScale", uvScale);
        shader_.setVec4("tint", tint);
        tex.bind(0);
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

private:
    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
};
