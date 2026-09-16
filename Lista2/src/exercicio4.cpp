// =============================================================================
// EXERCICIO 4 - Lista 2 -
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
    int winW = 800, winH = 600;
    GLFWwindow* window = glbCreateWindow("Exercicio 4 - Viewport no quadrante superior direito", winW, winH);
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Triangulo
    std::vector<float> vertices = {
        -0.6f, -0.6f, 0.0f,
         0.6f, -0.6f, 0.0f,
         0.0f,  0.6f, 0.0f,
    };
    Mesh mesh = glbCreateMesh(vertices, false);

    while (!glfwWindowShouldClose(window)) {

        // A janela inteira fica com a cor de fundo
        glViewport(0, 0, winW, winH);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // SUPERIOR DIREITO (metade da largura, metade da altura, deslocado
        // para x = w/2 e y = h/2
        glViewport(winW / 2, winH / 2, winW / 2, winH / 2);

        shader.use();
        shader.setMat4("uProjection", projection);
        shader.setVec4("uColor", 0.85f, 0.15f, 0.85f, 1.0f);

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
