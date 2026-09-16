// =============================================================================
// EXERCICIO 2 - Lista 2 - 
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

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 2 - Janela do mundo em pixels (Y invertido)", 800, 600);
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // Janela do mundo xmin=0, xmax=800, ymin=600, ymax=0
    // (bottom=600, top=0 -> eixo Y fica de cabeca para baixo, como uma tela)
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    // Retangulo desenhado em coordenadas de PIXEL. Como y cresce pra baixo,
    // este retangulo aparece perto do canto SUPERIOR esquerdo da tela.
    std::vector<float> vertices = {
        100.0f, 100.0f, 0.0f,
        300.0f, 100.0f, 0.0f,
        300.0f, 250.0f, 0.0f,

        100.0f, 100.0f, 0.0f,
        300.0f, 250.0f, 0.0f,
        100.0f, 250.0f, 0.0f,
    };
    Mesh mesh = glbCreateMesh(vertices, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setMat4("uProjection", projection);
        shader.setVec4("uColor", 0.95f, 0.45f, 0.15f, 1.0f);

        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(mesh);
    glfwTerminate();
    return 0;
}
