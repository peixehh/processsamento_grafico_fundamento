// EXERCICIO 1 - Letra a) - Lista 1 
//
// Desenhe 2 triangulos na tela, apenas com o poligono preenchido.

#include "GLBase.h"
#include "Shader.h"
#include <vector>

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 position;
void main() { gl_Position = vec4(position, 1.0); }
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 color;
uniform vec4 uColor;
void main() { color = uColor; }
)";

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 1a - Dois triangulos (preenchido)");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // Dois triangulos
    std::vector<float> vertices = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.0f,  0.0f, 0.0f,

         0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
    };
    Mesh mesh = glbCreateMesh(vertices, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setVec4("uColor", 0.25f, 0.55f, 0.95f, 1.0f);
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
