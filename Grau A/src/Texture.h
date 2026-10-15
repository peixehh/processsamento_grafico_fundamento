#pragma once
#include <glad/gl.h>
#include <iostream>
#include <string>

#include "stb_image.h"

// Textura 2D carregada com stb_image. Pixel art -> filtro NEAREST.
struct Texture
{
    GLuint id = 0;
    int w = 0, h = 0;

    // repeatX GL_REPEAT na horizontal (camadas de parallax), vertical sempre CLAMP.
    bool load(const std::string& path, bool repeatX = false)
    {
        int ch = 0;
        stbi_set_flip_vertically_on_load(false);  // UV (0,0) = canto superior esquerdo da imagem
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4);
        if (!data)
        {
            std::cerr << "Nao consegui carregar a imagem: " << path << "\n";
            return false;
        }
        upload(data, repeatX);
        stbi_image_free(data);
        return true;
    }

    // Textura 1x1 branca serve para desenhar retangulos coloridos com o mesmo shader.
    void makeWhite()
    {
        unsigned char px[4] = { 255, 255, 255, 255 };
        w = h = 1;
        upload(px, false);
    }

    void bind(int unit = 0) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

private:
    void upload(const unsigned char* data, bool repeatX)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatX ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
};
