#version 330 core

smooth in vec2      fs_texc;
smooth in vec4      fs_color;
smooth out vec4     final_color;

uniform sampler2D   texture;
uniform float       radius;

// Performs a vertical blur by sampling 9 pixels.
void main(void)
{
    vec4 sum = vec4(0.0);

    // blur in y (horizontal)
    // take nine samples, with the distance 'radius' between them
    sum += texture2D(texture, vec2(fs_texc.x - 4.0*radius, fs_texc.y)) * 0.05;
    sum += texture2D(texture, vec2(fs_texc.x - 3.0*radius, fs_texc.y)) * 0.09;
    sum += texture2D(texture, vec2(fs_texc.x - 2.0*radius, fs_texc.y)) * 0.12;
    sum += texture2D(texture, vec2(fs_texc.x - radius, fs_texc.y))     * 0.15;
    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y))              * 0.16;
    sum += texture2D(texture, vec2(fs_texc.x + radius, fs_texc.y))     * 0.15;
    sum += texture2D(texture, vec2(fs_texc.x + 2.0*radius, fs_texc.y)) * 0.12;
    sum += texture2D(texture, vec2(fs_texc.x + 3.0*radius, fs_texc.y)) * 0.09;
    sum += texture2D(texture, vec2(fs_texc.x + 4.0*radius, fs_texc.y)) * 0.05;

    final_color = sum * fs_color;
}