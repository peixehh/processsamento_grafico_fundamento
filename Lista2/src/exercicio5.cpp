// =============================================================================
// EXERCICIO 5 - Lista 2 -S
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
    GLFWwindow* window = glbCreateWindow("Exercicio 5 - Mesma cena nos 4 quadrantes", winW, winH);
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    std::vector<float> vertices = {
        -0.6f, -0.6f, 0.0f,
         0.6f, -0.6f, 0.0f,
         0.0f,  0.6f, 0.0f,
    };
    Mesh mesh = glbCreateMesh(vertices, false);

    // Os 4 quadrantes, como (x, y, largura, altura) do viewport
    int halfW = winW / 2, halfH = winH / 2;
    int quadrants[4][4] = {
        {0,     halfH, halfW, halfH},  // superior esquerdo
        {halfW, halfH, halfW, halfH},  // superior direito
        {0,     0,     halfW, halfH},  // inferior esquerdo
        {halfW, 0,     halfW, halfH},  // inferior direito
    };

    while (!glfwWindowShouldClose(window)) {
        glViewport(0, 0, winW, winH);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setMat4("uProjection", projection);
        shader.setVec4("uColor", 0.85f, 0.15f, 0.85f, 1.0f);

        glBindVertexArray(mesh.VAO);
        for (auto& q : quadrants) {
            glViewport(q[0], q[1], q[2], q[3]);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(mesh);
    glfwTerminate();
    return 0;
}
