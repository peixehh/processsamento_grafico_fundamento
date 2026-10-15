#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

// Compila e linka um programa de shader a partir de dois arquivos de texto.
class Shader
{
public:
    bool load(const std::string& vertPath, const std::string& fragPath)
    {
        GLuint vs = compile(GL_VERTEX_SHADER, readFile(vertPath), vertPath);
        GLuint fs = compile(GL_FRAGMENT_SHADER, readFile(fragPath), fragPath);
        if (!vs || !fs) return false;

        id_ = glCreateProgram();
        glAttachShader(id_, vs);
        glAttachShader(id_, fs);
        glLinkProgram(id_);

        GLint ok = 0;
        glGetProgramiv(id_, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetProgramInfoLog(id_, 1024, nullptr, log);
            std::cerr << "Erro ao linkar shader: " << log << "\n";
            return false;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return true;
    }

    void use() const { glUseProgram(id_); }

    void setInt(const char* n, int v)              { glUniform1i(loc(n), v); }
    void setVec2(const char* n, const glm::vec2& v) { glUniform2fv(loc(n), 1, glm::value_ptr(v)); }
    void setVec4(const char* n, const glm::vec4& v) { glUniform4fv(loc(n), 1, glm::value_ptr(v)); }
    void setMat4(const char* n, const glm::mat4& m) { glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(m)); }

private:
    GLint loc(const char* name)
    {
        auto it = cache_.find(name);
        if (it != cache_.end()) return it->second;
        GLint l = glGetUniformLocation(id_, name);
        cache_[name] = l;
        return l;
    }

    static std::string readFile(const std::string& path)
    {
        std::ifstream f(path);
        if (!f) { std::cerr << "Nao consegui abrir o shader: " << path << "\n"; return ""; }
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static GLuint compile(GLenum type, const std::string& src, const std::string& name)
    {
        if (src.empty()) return 0;
        GLuint s = glCreateShader(type);
        const char* c = src.c_str();
        glShaderSource(s, 1, &c, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetShaderInfoLog(s, 1024, nullptr, log);
            std::cerr << "Erro ao compilar " << name << ": " << log << "\n";
            return 0;
        }
        return s;
    }

    GLuint id_ = 0;
    std::unordered_map<std::string, GLint> cache_;
};
