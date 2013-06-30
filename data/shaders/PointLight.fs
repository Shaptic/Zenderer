#version 330 core

smooth  in  vec2    fs_texc;        // Texture coordinates from VS
smooth  in  vec4    fs_color;

uniform int         scr_height;     // Screen height
uniform vec2        light_pos;      // Light position
uniform vec3        light_col;      // Light color
uniform vec3        light_att;      // Light attenuation
uniform float       light_brt;      // Light brightness

uniform sampler2D   geometry;       // texture with scene rendered to it

smooth  out vec4    out_color;      // Output color

void main()
{
    // Invert the fragment's y-value (I have no clue why this is necessary)
    vec2 pixel      = gl_FragCoord.xy;
    pixel.y         = scr_height - pixel.y;
    
    // Calculate distance to light from fragment.
    vec2 light_vec  = light_pos  - pixel;
    float dist      = length(light_vec);

    // Calculate attenuation, or light influence factor.
    float att       = 1.0 / ( light_att.x + 
                            ( light_att.y * dist) + 
                            ( light_att.z * dist * dist));

    // Final fragment color is the light color * attenuation * brightness.
    out_color       = vec4(light_col, 1.0) * vec4(att, att, att, 1.0);
    out_color       = fs_color * vec4(out_color.rgb * light_brt, 1.0);
    //out_color      *= texture2D(geometry, fs_texc);
}
