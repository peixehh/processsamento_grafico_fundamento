#version 330 core
// Vertex shader do texto (aula Buffers) um unico atributo vec4 por vertice,
// xy = posicao do caractere na tela e zw = coordenada de textura do glifo.
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection;   // ortografica de tela (origem no canto superior esquerdo)

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
