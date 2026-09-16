// =============================================================================
// EXERCICIO 3 - Lista 2 - 
// =============================================================================

#include "GLBase.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 uProjection;
void main() { gl_Position = uProjection * vec4(position, 1.0); }
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 color;
uniform vec4 uColor;
void main() { color = uColor; }
)";

// Gera um quadrado (2 triangulos) com centro (cx, cy) e lado "size", em
// coordenadas de PIXEL
std::vector<float> makeSquare(float cx, float cy, float size) {
    float h = size / 2.0f;
    return {
        cx - h, cy - h, 0.0f,
        cx + h, cy - h, 0.0f,
        cx + h, cy + h, 0.0f,

        cx - h, cy - h, 0.0f,
        cx + h, cy + h, 0.0f,
        cx - h, cy + h, 0.0f,
    };
}

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 3 - Posicionando objetos em coordenadas de tela", 800, 600);
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // Mesma janela do mundo do exercicio 2 0..800 x, 600..0 y (Y invertido)
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    // Cinco quadrados em posicoes de referencia da tela cada um usa
    // diretamente a coordenada de pixel de onde deve aparecer
    Mesh topLeft     = glbCreateMesh(makeSquare(60.0f,  60.0f,  80.0f), false);   // canto superior esquerdo
    Mesh topRight    = glbCreateMesh(makeSquare(740.0f, 60.0f,  80.0f), false);   // canto superior direito
    Mesh bottomLeft  = glbCreateMesh(makeSquare(60.0f,  540.0f, 80.0f), false);   // canto inferior esquerdo
    Mesh bottomRight = glbCreateMesh(makeSquare(740.0f, 540.0f, 80.0f), false);   // canto inferior direito
    Mesh center      = glbCreateMesh(makeSquare(400.0f, 300.0f, 100.0f), false);  // centro da tela

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setMat4("uProjection", projection);

        shader.setVec4("uColor", 0.95f, 0.3f, 0.3f, 1.0f);
        glBindVertexArray(topLeft.VAO);
        glDrawArrays(GL_TRIANGLES, 0, topLeft.vertexCount);

        shader.setVec4("uColor", 0.3f, 0.95f, 0.4f, 1.0f);
        glBindVertexArray(topRight.VAO);
        glDrawArrays(GL_TRIANGLES, 0, topRight.vertexCount);

        shader.setVec4("uColor", 0.3f, 0.6f, 0.95f, 1.0f);
        glBindVertexArray(bottomLeft.VAO);
        glDrawArrays(GL_TRIANGLES, 0, bottomLeft.vertexCount);

        shader.setVec4("uColor", 0.95f, 0.85f, 0.2f, 1.0f);
        glBindVertexArray(bottomRight.VAO);
        glDrawArrays(GL_TRIANGLES, 0, bottomRight.vertexCount);

        shader.setVec4("uColor", 0.85f, 0.4f, 0.95f, 1.0f);
        glBindVertexArray(center.VAO);
        glDrawArrays(GL_TRIANGLES, 0, center.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(topLeft);
    glbDestroyMesh(topRight);
    glbDestroyMesh(bottomLeft);
    glbDestroyMesh(bottomRight);
    glbDestroyMesh(center);
    glfwTerminate();
    return 0;
}
