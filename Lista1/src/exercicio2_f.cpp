// EXERCICIO 2 - Letra f) DESAFIO 2 - Lista 1 -
//
// DESAFIO 2: desenhe uma espiral, utilizando a equacao parametrica do
// circulo (com raio crescente) para gerar os vertices

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

// Espiral de Arquimedes
std::vector<float> generateSpiral(float cx, float cy, float a, float turns, int samples) {
    std::vector<float> verts;
    float maxTheta = turns * 2.0f * (float)M_PI;
    for (int i = 0; i <= samples; ++i) {
        float theta = ((float)i / (float)samples) * maxTheta;
        float r = a * theta;
        verts.push_back(cx + r * cosf(theta));  // equacao parametrica do circulo
        verts.push_back(cy + r * sinf(theta));  // equacao parametrica do circulo
        verts.push_back(0.0f);
    }
    return verts;
}

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 2f - DESAFIO 2: Espiral");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    std::vector<float> data = generateSpiral(0.0f, 0.0f, 0.045f, 3.0f, 300);
    Mesh mesh = glbCreateMesh(data, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setVec4("uColor", 0.4f, 0.8f, 1.0f, 1.0f);
        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_LINE_STRIP, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(mesh);
    glfwTerminate();
    return 0;
}
