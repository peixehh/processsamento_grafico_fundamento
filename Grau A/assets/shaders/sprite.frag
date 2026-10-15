#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D tex_buffer;   // pixels da textura (aula Texturas)
uniform vec4 tint;              // cor multiplicada (iluminacao dia/noite, dano, transparencia)

void main()
{
    vec4 c = texture(tex_buffer, vUV) * tint;
    if (c.a < 0.01) discard;
    FragColor = c;
}
