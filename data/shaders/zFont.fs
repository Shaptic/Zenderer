#version 330 core

uniform sampler2D   texture;

smooth  in  vec2    fs_texc;
smooth  in  vec4    fs_color;
        out vec4    out_color;

uniform vec2    offset;

void main()
{
    out_color = texture2D(texture, fs_texc).rrrr * fs_color;
}

