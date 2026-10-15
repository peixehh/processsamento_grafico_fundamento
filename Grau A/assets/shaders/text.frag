#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D tex_buffer;   // glifo da FreeType 1 canal (GL_RED) com a cobertura de cada pixel
uniform vec4 textColor;

void main()
{
    color = vec4(textColor.rgb, textColor.a * texture(tex_buffer, TexCoords).r);
}
