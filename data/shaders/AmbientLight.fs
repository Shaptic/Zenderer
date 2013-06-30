#version 330 core

smooth  in  vec2    fs_texc;    // Texture coordinates from VS
smooth  in  vec4    fs_color;

uniform vec3        light_col;  // Light color
uniform float       light_brt;  // Light brightness

uniform sampler2D   geometry;   // texture with scene rendered to it

smooth  out vec4    out_color;  // Output color

void main()
{
    out_color   = light_brt * vec4(light_col, 1.0);
    out_color  *= fs_color;// * texture2D(geometry, fs_texc);
}
