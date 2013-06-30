#version 330 core

uniform sampler2D   texture;
uniform vec2        tc_offset;

smooth  in  vec2 fs_texc;
smooth  in  vec4 fs_color;
        out vec4 out_color;

void main()
{
    out_color = texture2D(texture, fs_texc + tc_offset) * fs_color;
}
