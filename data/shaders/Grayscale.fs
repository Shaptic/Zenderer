#version 330 core

uniform sampler2D   texture;

smooth  in  vec2    fs_texc;
smooth  in  vec4    fs_color;
        out vec4    out_color;

void main()
{
    vec4 color = texture2D(texture, fs_texc);
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    out_color  = vec4(color.rgb * gray, color.a);
}

