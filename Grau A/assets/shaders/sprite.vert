#version 330 core
// Vertex shader unico do jogo todo objeto 2D e um quad unitario (0..1)
// posicionado por uma matriz de modelo e projetado por uma matriz ortografica.
layout (location = 0) in vec2 aPos;   // posicao do vertice no quad unitario
layout (location = 1) in vec2 aUV;    // coordenada de textura

uniform mat4 projection;  // ortografica (mundo ou tela)
uniform mat4 model;       // translacao * escala do objeto
uniform vec2 uvOffset;    // deslocamento no spritesheet / camada de parallax
uniform vec2 uvScale;     // tamanho da celula (dx, dy) no spritesheet

out vec2 vUV;

void main()
{
    vUV = aUV * uvScale + uvOffset;
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}
