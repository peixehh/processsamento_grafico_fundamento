// EXERCÍCIO 4 - Lista 1 -
//
// Faça um desenho em um papel quadriculado (pode ser no computador mesmo) e
//  reproduza-o utilizando primitivas em OpenGL. Neste exercício você poderá
//  criar mais de um VAO e fazer mais de uma chamada de desenho para poder
//  utilizar primitivas diferentes, se necessário

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
    GLFWwindow* window = glbCreateWindow("Exercicio 4 - Desenho com multiplos VAOs (exemplo: casinha)");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    std::cout << "Exercicio 4 Desenho com multiplos VAOs casinha\n";

    // VAO 0 corpo da casa (quadrado -> 2 triangulos)
    std::vector<float> body = {
        -0.4f, -0.5f, 0.0f,
         0.4f, -0.5f, 0.0f,
         0.4f,  0.1f, 0.0f,

        -0.4f, -0.5f, 0.0f,
         0.4f,  0.1f, 0.0f,
        -0.4f,  0.1f, 0.0f,
    };
    Mesh meshBody = glbCreateMesh(body, false);

    // VAO 1 telhado (triangulo)
    std::vector<float> roof = {
        -0.5f, 0.1f, 0.0f,
         0.5f, 0.1f, 0.0f,
         0.0f, 0.55f, 0.0f,
    };
    Mesh meshRoof = glbCreateMesh(roof, false);

    // VAO 2 porta (retangulo -> 2 triangulos)
    std::vector<float> door = {
        -0.08f, -0.5f, 0.0f,
         0.08f, -0.5f, 0.0f,
         0.08f, -0.15f, 0.0f,

        -0.08f, -0.5f, 0.0f,
         0.08f, -0.15f, 0.0f,
        -0.08f, -0.15f, 0.0f,
    };
    Mesh meshDoor = glbCreateMesh(door, false);

    // VAO 3 janela - vidro (quadrado -> 2 triangulos)
    std::vector<float> windowFill = {
        -0.30f, -0.05f, 0.0f,
        -0.10f, -0.05f, 0.0f,
        -0.10f,  0.05f, 0.0f,

        -0.30f, -0.05f, 0.0f,
        -0.10f,  0.05f, 0.0f,
        -0.30f,  0.05f, 0.0f,
    };
    Mesh meshWindow = glbCreateMesh(windowFill, false);

    // VAO 4 janela - cruzeta 
    std::vector<float> windowCross = {
        -0.20f, -0.05f, 0.0f,
        -0.20f,  0.05f, 0.0f,

        -0.30f,  0.0f, 0.0f,
        -0.10f,  0.0f, 0.0f,
    };
    Mesh meshCross = glbCreateMesh(windowCross, false);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        shader.setVec4("uColor", 0.85f, 0.85f, 0.85f, 1.0f);  // corpo
        glBindVertexArray(meshBody.VAO);
        glDrawArrays(GL_TRIANGLES, 0, meshBody.vertexCount);

        shader.setVec4("uColor", 0.75f, 0.15f, 0.1f, 1.0f);  // telhado
        glBindVertexArray(meshRoof.VAO);
        glDrawArrays(GL_TRIANGLES, 0, meshRoof.vertexCount);

        shader.setVec4("uColor", 0.4f, 0.22f, 0.05f, 1.0f);  // porta
        glBindVertexArray(meshDoor.VAO);
        glDrawArrays(GL_TRIANGLES, 0, meshDoor.vertexCount);

        shader.setVec4("uColor", 0.6f, 0.85f, 1.0f, 1.0f);  // janela (vidro)
        glBindVertexArray(meshWindow.VAO);
        glDrawArrays(GL_TRIANGLES, 0, meshWindow.vertexCount);

        shader.setVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f);  // cruzeta da janela
        glBindVertexArray(meshCross.VAO);
        glDrawArrays(GL_LINES, 0, meshCross.vertexCount);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glbDestroyMesh(meshBody);
    glbDestroyMesh(meshRoof);
    glbDestroyMesh(meshDoor);
    glbDestroyMesh(meshWindow);
    glbDestroyMesh(meshCross);
    glfwTerminate();
    return 0;
}
