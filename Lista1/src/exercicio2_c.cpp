// EXERCICIO 2 - Letra c) - Lista 1 -
//
// Utilizando a equacao parametrica do circulo para gerar os vertices,
// desenhe um pac-man

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

// Gera um leque de um arco de circulo
std::vector<float> generateArcFan(float cx, float cy, float radius,
                                   int segments, float startDeg, float endDeg) {
    std::vector<float> verts = {cx, cy, 0.0f};  // centro do leque

    float startRad = startDeg * (float)M_PI / 180.0f;
    float endRad = endDeg * (float)M_PI / 180.0f;

    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float theta = startRad + t * (endRad - startRad);
        float x = cx + radius * cosf(theta);   // equacao parametrica do circulo
        float y = cy + radius * sinf(theta);   // equacao parametrica do circulo
        verts.push_back(x);
        verts.push_back(y);
        verts.push_back(0.0f);
    }
    return verts;
}

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 2c - Pac-Man");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // VAO 0: corpo do pac-man (circulo com uma boca -> arco de 35 a 325 graus)
    std::vector<float> body = generateArcFan(0.0f, 0.0f, 0.6f, 40, 35.0f, 325.0f);
    Mesh meshBody = glbCreateMesh(body, false);

    // VAO 1: olho circulo pequeno completo
    std::vector<float> eye = generateArcFan(-0.05f, 0.30f, 0.07f, 20, 0.0f, 360.0f);
    Mesh meshEye = glbCreateMesh(eye, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        shader.setVec4("uColor", 1.0f, 0.85f, 0.0f, 1.0f);  // corpo amarelo
        glBindVertexArray(meshBody.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshBody.vertexCount);

        shader.setVec4("uColor", 0.0f, 0.0f, 0.0f, 1.0f);  // olho preto
        glBindVertexArray(meshEye.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshEye.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(meshBody);
    glbDestroyMesh(meshEye);
    glfwTerminate();
    return 0;
}
