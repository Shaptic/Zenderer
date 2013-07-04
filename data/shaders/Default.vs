#version 420 core

layout(location=0) in vec3  vs_vertex;
layout(location=1) in vec2  vs_texc;
layout(location=2) in vec4  vs_color;

uniform mat4    proj;
uniform mat4    mv;
uniform float   x_shear;

smooth  out vec3 fs_vertex;
smooth  out vec4 fs_color;
smooth  out vec2 fs_texc;

void main()
{
    gl_Position = proj * mv * vec4(vs_vertex, 1.0);

    fs_color    = vs_color;
    fs_texc     = vs_texc;
    fs_vertex   = vs_vertex;
}
