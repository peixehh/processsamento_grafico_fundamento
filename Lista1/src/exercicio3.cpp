// EXERCÍCIO 3 - Lista 1 -
//
// Considerando o triângulo formado pelos vértices P1, P2 e P3, respectivamente
//  com as cores vermelho, verde e azul:
//    a) Descreva uma possível configuração dos buffers (VBO, VAO) para representá-lo.
//    b) Como estes atributos seriam identificados no vertex shader?
//  Agora implemente


#include "GLBase.h"
#include "Shader.h"
#include <vector>

// Vertex shader com 2 atributos posição (location 0) e cor (location 1)
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(position, 1.0);
    vertexColor = color;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 vertexColor;
out vec4 color;

void main()
{
    color = vec4(vertexColor, 1.0);
}
)";

int main() {
    GLFWwindow* window = glbCreateWindow("Exercicio 3 - Triangulo colorido (VBO/VAO)");
    if (!window) return -1;
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });

    Shader shader(vertexSrc, fragmentSrc);

    std::cout << "Exercicio 3 Triangulo com VBO intercalado + VAO 2 atributos\n";

    // P1 (topo, vermelho), P2 (esq. inferior, verde), P3 (dir. inferior, azul)
    std::vector<float> tri = {
        // posicao            // cor
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // P1 - vermelho
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // P2 - verde
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  // P3 - azul
    };
    Mesh mesh = glbCreateMesh(tri, /*hasColor=*/true);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
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
