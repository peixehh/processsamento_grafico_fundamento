// GLBase.h
// Infraestrutura comum (criação de janela GLFW/GLAD + helpers de VAO/VBO).
// Isto NÃO contém nenhuma lógica de exercício — é só o "boilerplate" que
// toda janela OpenGL precisa, para não repetir isso em cada arquivo.
#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Cria uma janela com contexto OpenGL 3.3 core + inicializa o GLAD.
// Retorna nullptr em caso de erro (mensagem já impressa no console).
inline GLFWwindow* glbCreateWindow(const char* title, int width = 800, int height = 800) {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o GLAD" << std::endl;
        return nullptr;
    }

    glViewport(0, 0, width, height);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glLineWidth(2.0f);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    return window;
}

// ---------------------------------------------------------------------------
// Mesh: agrupa um VAO + VBO já configurados
// ---------------------------------------------------------------------------
struct Mesh {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLsizei vertexCount = 0;
};

// hasColor = false -> layout por vertice: x,y,z                 (so location 0)
// hasColor = true  -> layout por vertice: x,y,z, r,g,b           (location 0 e 1)
inline Mesh glbCreateMesh(const std::vector<float>& data, bool hasColor) {
    Mesh mesh;
    mesh.vertexCount = hasColor ? (GLsizei)(data.size() / 6) : (GLsizei)(data.size() / 3);

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    if (!hasColor) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    } else {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);
    return mesh;
}

inline void glbDestroyMesh(Mesh& mesh) {
    if (mesh.VBO) glDeleteBuffers(1, &mesh.VBO);
    if (mesh.VAO) glDeleteVertexArrays(1, &mesh.VAO);
    mesh = Mesh{};
}
