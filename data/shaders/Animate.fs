#version 330 core

uniform sampler2D   texture;
uniform float       tc_offset;
uniform vec2        tc_start;

smooth  in  vec2 fs_texc;
smooth  in  vec4 fs_color;
        out vec4 out_color;

void main()
{
    vec2 coord= vec2(tc_start.s + (fs_texc.s * tc_offset), tc_start.t + fs_texc.t);
    out_color = texture2D(texture, coord) * fs_color;
}
