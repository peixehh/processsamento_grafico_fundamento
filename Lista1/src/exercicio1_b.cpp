// EXERCICIO 1 - Letra b) - Lista 1 - 
//
// Desenhe 2 triangulos na tela, apenas com o contorno.

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
    GLFWwindow* window = glbCreateWindow("Exercicio 1b - Dois triangulos (contorno)");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // Dois triangulos, cada um em seu proprio VBO/VAO para poder ser desenhado como contorno individualmente
    std::vector<float> triA = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.0f,  0.0f, 0.0f,
    };
    std::vector<float> triB = {
         0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
    };
    Mesh meshA = glbCreateMesh(triA, false);
    Mesh meshB = glbCreateMesh(triB, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f);

        glBindVertexArray(meshA.VAO);
        glDrawArrays(GL_LINE_LOOP, 0, meshA.vertexCount);
        glBindVertexArray(meshB.VAO);
        glDrawArrays(GL_LINE_LOOP, 0, meshB.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(meshA);
    glbDestroyMesh(meshB);
    glfwTerminate();
    return 0;
}
