// Shader.h
// Classe utilitária simples para compilar e linkar um vertex + fragment
// shader a partir de strings de código-fonte GLSL. Baseado no padrão usado
// nos slides da disciplina (Introdução a OpenGL/Shaders) e no LearnOpenGL.
#pragma once

#include <glad/gl.h>
#include <string>
#include <iostream>

class Shader {
public:
    GLuint ID = 0;

    Shader(const char* vertexSrc, const char* fragmentSrc) {
        GLuint vertex = compile(GL_VERTEX_SHADER, vertexSrc, "VERTEX");
        GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSrc, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkLinkErrors(ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader() {
        if (ID) glDeleteProgram(ID);
    }

    void use() const { glUseProgram(ID); }

    void setVec4(const char* name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
    }

private:
    static GLuint compile(GLenum type, const char* src, const char* tag) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[ERRO] Falha ao compilar shader (" << tag << "):\n"
                      << infoLog << std::endl;
        }
        return shader;
    }

    static void checkLinkErrors(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(program, 1024, nullptr, infoLog);
            std::cerr << "[ERRO] Falha ao linkar programa de shader:\n"
                      << infoLog << std::endl;
        }
    }
};
