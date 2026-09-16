// =============================================================================
// EXERCICIO 1 - Lista 2 - 
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
    GLFWwindow* window = glbCreateWindow("Exercicio 1 - Janela do mundo [-10, 10]", 800, 800);
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // Janela do mundo xmin=-10, xmax=10, ymin=-10, ymax=10
    glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);

    // Triangulo definido em UNIDADES DO MUNDO
    std::vector<float> vertices = {
        -4.0f, -4.0f, 0.0f,
         4.0f, -4.0f, 0.0f,
         0.0f,  4.0f, 0.0f,
    };
    Mesh mesh = glbCreateMesh(vertices, false);

    std::vector<float> refSquare = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    Mesh meshRef = glbCreateMesh(refSquare, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setMat4("uProjection", projection);

        shader.setVec4("uColor", 0.25f, 0.55f, 0.95f, 1.0f);
        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

        shader.setVec4("uColor", 1.0f, 0.85f, 0.1f, 1.0f);
        glBindVertexArray(meshRef.VAO);
        glDrawArrays(GL_TRIANGLES, 0, meshRef.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(mesh);
    glbDestroyMesh(meshRef);
    glfwTerminate();
    return 0;
}
