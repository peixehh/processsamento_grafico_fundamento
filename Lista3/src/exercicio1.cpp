// =============================================================================
// EXERCICIO 1 - Lista 3 - Processamento Grafico: Fundamentos (UNISINOS)
// "Criando Triangulos a partir do Clique do Mouse"
//
// Requisitos do enunciado:
//    1) Ao clicar na tela, voce esta criando apenas 1 vertice
//    2) A cada 3 vertices criados, voce cria um triangulo
//    3) Para cada novo triangulo criado, voce deve usar uma cor nova
//
// Dica seguida (conforme o enunciado): a janela do mundo usa projecao
// ortografica com as MESMAS dimensoes da janela GLFW (800x600), entao
// 1 unidade do mundo = 1 pixel, e a posicao do clique do mouse (que o
// GLFW ja reporta em pixels, com origem no canto superior esquerdo) pode
// ser usada DIRETO como posicao do vertice, sem nenhuma conversao.
//
// Cada triangulo clicado ganha seu proprio VAO/VBO (diversos VAOs, uma
// das duas opcoes sugeridas pelo enunciado).
//
// Programa INDEPENDENTE: gera direto o resultado deste exercicio.
// CONTROLES:
//   Clique esquerdo -> adiciona um vertice (a cada 3, fecha um triangulo)
//   ESC              -> fecha a janela
// =============================================================================

#include "GLBase.h"
#include <cstdio>
#include <cstdlib>
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * vec4(position, 1.0);
    gl_PointSize = 8.0;
}
)";

const char* fragmentSrc = R"(
#version 330 core
out vec4 color;
uniform vec4 uColor;
void main() { color = uColor; }
)";

// Paleta de cores: cada triangulo novo usa a proxima cor da lista (e volta
// pro comeco se passar do fim), conforme pedido pelo exercicio.
const std::vector<glm::vec3> PALETTE = {
    {0.90f, 0.30f, 0.30f}, {0.30f, 0.85f, 0.40f}, {0.30f, 0.55f, 0.95f},
    {0.95f, 0.80f, 0.20f}, {0.80f, 0.35f, 0.95f}, {0.25f, 0.85f, 0.85f},
};

// Estado global: vertices ainda sendo clicados (0, 1 ou 2) e a lista de
// triangulos ja fechados (cada um com seu proprio VAO/VBO e cor fixa).
std::vector<glm::vec2> g_pendingVerts;
std::vector<Mesh> g_triangles;
std::vector<glm::vec3> g_triangleColors;
int g_nextColor = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);  // ja vem em pixels, origem no canto superior esquerdo
    g_pendingVerts.push_back(glm::vec2((float)x, (float)y));

    std::cout << "Vertice " << g_pendingVerts.size() << "/3 criado em ("
              << x << ", " << y << ")" << std::endl;

    if (g_pendingVerts.size() == 3) {
        std::vector<float> data;
        for (auto& v : g_pendingVerts) {
            data.push_back(v.x);
            data.push_back(v.y);
            data.push_back(0.0f);
        }
        g_triangles.push_back(glbCreateMesh(data, false));
        g_triangleColors.push_back(PALETTE[g_nextColor % PALETTE.size()]);
        g_nextColor++;

        std::cout << "Triangulo " << g_triangles.size() << " criado!" << std::endl;
        g_pendingVerts.clear();
    }
}

int main() {
    GLFWwindow* window = glbCreateWindow("Lista 3 - Triangulos por clique do mouse", 800, 600);
    if (!window) return -1;

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
    });
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    Shader shader(vertexSrc, fragmentSrc);

    std::cout << "=== Lista 3: clique na janela para criar vertices ===\n"
              << "A cada 3 cliques um triangulo novo e criado, com uma cor nova.\n"
              << "ESC -> fechar\n";

    // Janela do mundo com as mesmas dimensoes da janela GLFW (800x600),
    // exatamente como a dica do enunciado sugere: 1 unidade = 1 pixel.
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setMat4("uProjection", projection);

        // Desenha todos os triangulos ja fechados, cada um com sua cor.
        for (size_t i = 0; i < g_triangles.size(); ++i) {
            glm::vec3 c = g_triangleColors[i];
            shader.setVec4("uColor", c.r, c.g, c.b, 1.0f);
            glBindVertexArray(g_triangles[i].VAO);
            glDrawArrays(GL_TRIANGLES, 0, g_triangles[i].vertexCount);
        }

        // Desenha os vertices ainda sendo posicionados (0, 1 ou 2) como
        // pontos brancos, so pra dar feedback visual antes do triangulo
        // fechar.
        if (!g_pendingVerts.empty()) {
            std::vector<float> data;
            for (auto& v : g_pendingVerts) {
                data.push_back(v.x);
                data.push_back(v.y);
                data.push_back(0.0f);
            }
            Mesh pending = glbCreateMesh(data, false);
            shader.setVec4("uColor", 1.0f, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(pending.VAO);
            glDrawArrays(GL_POINTS, 0, pending.vertexCount);
            glbDestroyMesh(pending);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& m : g_triangles) glbDestroyMesh(m);
    glfwTerminate();
    return 0;
}
