#version 330 core

uniform float       alpha;
uniform sampler2D   texture;
uniform vec2        tc_offset;

smooth  in  vec2 fs_texc;
smooth  in  vec4 fs_color;
        out vec4 out_color;

void main()
{
    vec4 final = texture2D(texture, fs_texc + tc_offset) * fs_color;
    out_color  = vec4(alpha * final.rgb, final.a);
}
