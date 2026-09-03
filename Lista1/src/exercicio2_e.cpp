// EXERCICIO 2 - Letra e) DESAFIO 1 - Lista 1 -
//
// DESAFIO 1: desenhe uma 'estrela', utilizando a equacao parametrica do
// circulo para gerar os vertices (alternando raio externo/interno)


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

// Estrela alterna raio externo/interno a cada segmento, usando a mesma
// equacao parametrica do circulo (x = cx + r*cos, y = cy + r*sin)
std::vector<float> generateStarFan(float cx, float cy, float outerR, float innerR, int points) {
    std::vector<float> verts = {cx, cy, 0.0f};
    int segments = points * 2;
    float startAngle = -90.0f;  // primeira ponta apontando para cima

    for (int i = 0; i <= segments; ++i) {
        float angleDeg = startAngle + i * (360.0f / segments);
        float rad = angleDeg * (float)M_PI / 180.0f;
        float r = (i % 2 == 0) ? outerR : innerR;
        verts.push_back(cx + r * cosf(rad));
        verts.push_back(cy + r * sinf(rad));
        verts.push_back(0.0f);
    }
    return verts;
}

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 2e - DESAFIO 1: Estrela");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    std::vector<float> data = generateStarFan(0.0f, 0.0f, 0.7f, 0.28f, 5);
    Mesh mesh = glbCreateMesh(data, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setVec4("uColor", 1.0f, 0.75f, 0.1f, 1.0f);
        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(mesh);
    glfwTerminate();
    return 0;
}
