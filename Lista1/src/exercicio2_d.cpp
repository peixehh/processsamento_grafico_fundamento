// EXERCICIO 2 - Letra d) - Lista 1 -
//
// Utilizando a equacao parametrica do circulo para gerar os vertices,
// desenhe uma fatia de pizza


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
    GLFWwindow* window = glbCreateWindow("Exercicio 2d - Fatia de pizza");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    // VAO 0 massa/queijo (arco de -25 a 25 graus)
    std::vector<float> slice = generateArcFan(0.0f, 0.3f, 0.8f, 16, -25.0f, 25.0f);
    Mesh meshSlice = glbCreateMesh(slice, false);

    // VAOs 1-3 pepperonis (pequenos circulos completos)
    std::vector<float> pep1 = generateArcFan(-0.05f, 0.15f, 0.045f, 12, 0.0f, 360.0f);
    std::vector<float> pep2 = generateArcFan(0.10f, -0.05f, 0.045f, 12, 0.0f, 360.0f);
    std::vector<float> pep3 = generateArcFan(-0.02f, -0.20f, 0.045f, 12, 0.0f, 360.0f);
    Mesh meshPep1 = glbCreateMesh(pep1, false);
    Mesh meshPep2 = glbCreateMesh(pep2, false);
    Mesh meshPep3 = glbCreateMesh(pep3, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        shader.setVec4("uColor", 0.95f, 0.80f, 0.35f, 1.0f);  // massa/queijo
        glBindVertexArray(meshSlice.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshSlice.vertexCount);

        shader.setVec4("uColor", 0.75f, 0.15f, 0.10f, 1.0f);  // pepperoni
        glBindVertexArray(meshPep1.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshPep1.vertexCount);
        glBindVertexArray(meshPep2.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshPep2.vertexCount);
        glBindVertexArray(meshPep3.VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, meshPep3.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(meshSlice);
    glbDestroyMesh(meshPep1);
    glbDestroyMesh(meshPep2);
    glbDestroyMesh(meshPep3);
    glfwTerminate();
    return 0;
}
