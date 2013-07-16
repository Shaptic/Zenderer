/**
 * @file
 *  Zenderer/CoreGraphics/ShaderFiles.hpp - Constants for raw shader data, 
 *  generated from a script to prevent distributing raw shader files in 
 *  public release binaries.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup CoreGraphics
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__SHADER_FILES_HPP
#define ZENDERER__CORE_GRAPHICS__SHADER_FILES_HPP

#include "Zenderer/Core/Types.hpp"

namespace zen
{
namespace gfxcore
{
    // Begin shader data.

    /// Created from data/shaders/AmbientLight.fs.
    static const string_t AMBIENTLIGHT_FS = string_t(
        "#version 330 core"

        "smooth  in  vec2    fs_texc;    // Texture coordinates from VS"
        "smooth  in  vec4    fs_color;"

        "uniform vec3        light_col;  // Light color"
        "uniform float       light_brt;  // Light brightness"

        "uniform sampler2D   geometry;   // texture with scene rendered to it"

        "smooth  out vec4    out_color;  // Output color"

        "void main()"
        "{"
        "    out_color   = light_brt * vec4(light_col, 1.0);"
        "    out_color  *= fs_color;// * texture2D(geometry, fs_texc);"
        "}"
    );

    /// Created from data/shaders/Animate.fs.
    static const string_t ANIMATE_FS = string_t(
        "#version 330 core"

        "uniform sampler2D   texture;"
        "uniform float       tc_offset;"
        "uniform vec2        tc_start;"

        "smooth  in  vec2 fs_texc;"
        "smooth  in  vec4 fs_color;"
        "        out vec4 out_color;"

        "void main()"
        "{"
        "    vec2 coord= vec2(tc_start.s + (fs_texc.s * tc_offset), tc_start.t + fs_texc.t);"
        "    out_color = texture2D(texture, coord) * fs_color;"
        "}"
    );

    /// Created from data/shaders/Default.fs.
    static const string_t DEFAULT_FS = string_t(
        "#version 330 core"

        "uniform sampler2D   texture;"
        "uniform vec2        tc_offset;"

        "smooth  in  vec2 fs_texc;"
        "smooth  in  vec4 fs_color;"
        "        out vec4 out_color;"

        "void main()"
        "{"
        "    out_color = texture2D(texture, fs_texc + tc_offset) * fs_color;"
        "}"
    );

    /// Created from data/shaders/Default.vs.
    static const string_t DEFAULT_VS = string_t(
        "#version 420 core"

        "layout(location=0) in vec3  vs_vertex;"
        "layout(location=1) in vec2  vs_texc;"
        "layout(location=2) in vec4  vs_color;"

        "uniform mat4    proj;"
        "uniform mat4    mv;"
        "uniform float   x_shear;"

        "smooth  out vec3 fs_vertex;"
        "smooth  out vec4 fs_color;"
        "smooth  out vec2 fs_texc;"

        "void main()"
        "{"
        "    gl_Position = proj * mv * vec4(vs_vertex, 1.0);"

        "    fs_color    = vs_color;"
        "    fs_texc     = vs_texc;"
        "    fs_vertex   = vs_vertex;"
        "}"
    );

    /// Created from data/shaders/Fade.fs.
    static const string_t FADE_FS = string_t(
        "#version 330 core"

        "uniform float       alpha;"
        "uniform sampler2D   texture;"
        "uniform vec2        tc_offset;"

        "smooth  in  vec2 fs_texc;"
        "smooth  in  vec4 fs_color;"
        "        out vec4 out_color;"

        "void main()"
        "{"
        "    vec4 final = texture2D(texture, fs_texc + tc_offset) * fs_color;"
        "    out_color  = vec4(alpha * final.rgb, final.a);"
        "}"
    );

    /// Created from data/shaders/GaussianBlur.vs.
    static const string_t GAUSSIANBLUR_VS = string_t(
        "#version 420 core"

        "layout(location=0) in vec2  vs_vertex;"
        "layout(location=1) in vec2  vs_texc;"
        "layout(location=2) in vec4  vs_color;"

        "uniform mat4 proj;"
        "uniform mat4 mv;"

        "smooth  out vec2 fs_vertex;"
        "smooth  out vec4 fs_color;"
        "smooth  out vec2 fs_texc;"

        "void main()"
        "{"
        "    gl_Position = proj * mv * vec4(vs_vertex, 1.0, 1.0);"

        "    fs_color    = vs_color;"
        "    fs_texc     = vs_texc;"
        "    fs_vertex   = sign(vs_vertex.xy) * 0.5 + 0.5;"
        "}"
    );

    /// Created from data/shaders/GaussianBlurH.fs.
    static const string_t GAUSSIANBLURH_FS = string_t(
        "#version 330 core"

        "smooth in vec2      fs_texc;"
        "smooth in vec4      fs_color;"
        "smooth out vec4     final_color;"

        "uniform sampler2D   texture;"
        "uniform float       radius;"

        "// Performs a vertical blur by sampling 9 pixels."
        "void main(void)"
        "{"
        "    vec4 sum = vec4(0.0);"

        "    // blur in y (horizontal)"
        "    // take nine samples, with the distance 'radius' between them"
        "    sum += texture2D(texture, vec2(fs_texc.x - 4.0*radius, fs_texc.y)) * 0.05;"
        "    sum += texture2D(texture, vec2(fs_texc.x - 3.0*radius, fs_texc.y)) * 0.09;"
        "    sum += texture2D(texture, vec2(fs_texc.x - 2.0*radius, fs_texc.y)) * 0.12;"
        "    sum += texture2D(texture, vec2(fs_texc.x - radius, fs_texc.y))     * 0.15;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y))              * 0.16;"
        "    sum += texture2D(texture, vec2(fs_texc.x + radius, fs_texc.y))     * 0.15;"
        "    sum += texture2D(texture, vec2(fs_texc.x + 2.0*radius, fs_texc.y)) * 0.12;"
        "    sum += texture2D(texture, vec2(fs_texc.x + 3.0*radius, fs_texc.y)) * 0.09;"
        "    sum += texture2D(texture, vec2(fs_texc.x + 4.0*radius, fs_texc.y)) * 0.05;"

        "    final_color = sum * fs_color;"
        ""
    );

    /// Created from data/shaders/GaussianBlurV.fs.
    static const string_t GAUSSIANBLURV_FS = string_t(
        "#version 330 core"

        "smooth in vec2      fs_texc;"
        "smooth in vec4      fs_color;"
        "smooth out vec4     final_color;"

        "uniform sampler2D   texture;"
        "uniform float       radius;"

        "// Performs a horizontal blur by sampling 9 pixels."
        "void main(void)"
        "{"
        "    vec4 sum = vec4(0.0);"

        "    // blur in y (vertical)"
        "    // take nine samples, with the distance 'radius' between them"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 4.0*radius))  * 0.05;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 3.0*radius))  * 0.09;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 2.0*radius))  * 0.12;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - radius))      * 0.15;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y))               * 0.16;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + radius))      * 0.15;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 2.0*radius))  * 0.12;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 3.0*radius))  * 0.09;"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 4.0*radius))  * 0.05;"

        "    final_color = sum * fs_color;"
        ""
    );

    /// Created from data/shaders/Grayscale.fs.
    static const string_t GRAYSCALE_FS = string_t(
        "#version 330 core"

        "uniform sampler2D   texture;"

        "smooth  in  vec2    fs_texc;"
        "smooth  in  vec4    fs_color;"
        "        out vec4    out_color;"

        "void main()"
        "{"
        "    vec4 color = texture2D(texture, fs_texc);"
        "    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));"
        "    out_color  = vec4(color.rgb * gray, color.a);"
        "}"
    );

    /// Created from data/shaders/PointLight.fs.
    static const string_t POINTLIGHT_FS = string_t(
        "#version 330 core"

        "smooth  in  vec2    fs_texc;        // Texture coordinates from VS"
        "smooth  in  vec4    fs_color;"

        "uniform int         scr_height;     // Screen height"
        "uniform vec2        light_pos;      // Light position"
        "uniform vec3        light_col;      // Light color"
        "uniform vec3        light_att;      // Light attenuation"
        "uniform float       light_brt;      // Light brightness"

        "uniform sampler2D   geometry;       // texture with scene rendered to it"

        "smooth  out vec4    out_color;      // Output color"

        "void main()"
        "{"
        "    // Invert the fragment's y-value (I have no clue why this is necessary)"
        "    vec2 pixel      = gl_FragCoord.xy;"
        "    pixel.y         = scr_height - pixel.y;"

        "    // Calculate distance to light from fragment."
        "    vec2 light_vec  = light_pos  - pixel;"
        "    float dist      = length(light_vec);"

        "    // Calculate attenuation, or light influence factor."
        "    float att       = 1.0 / ( light_att.x +"
        "                            ( light_att.y * dist) +"
        "                            ( light_att.z * dist * dist));"

        "    // Final fragment color is the light color * attenuation * brightness."
        "    out_color       = texture2D(geometry, fs_texc);"
        "    out_color      *= vec4(light_col, 1.0) * vec4(att, att, att, 1.0);"
        "    out_color       = fs_color * vec4(out_color.rgb * light_brt, 1.0);"
        "}"
    );

    /// Created from data/shaders/Ripple.fs.
    static const string_t RIPPLE_FS = string_t(
        "#version 330"

        "uniform float       rate;"
        "uniform float       time;   // time in seconds"
        "uniform sampler2D   tex;    // scene buffer"

        "smooth  in  vec2 fs_texc;"
        "smooth  in  vec4 fs_color;"
        "        out vec4 out_color;"

        "void main(void)"
        "{"
        "  vec2 pos  = -1.0 + 2.0 * fs_texc;//gl_FragCoord.xy / res;"
        "  float len = length(pos * max(0.2, time * rate));"
        "  vec2 uv   = fs_texc + (pos / len) * cos(len * 12.0 - time * 4.0) * 0.03;"

        "  out_color = texture2D(tex, uv) * fs_color;"
        "}"
    );

    /// Created from data/shaders/SpotLight.fs.
    static const string_t SPOTLIGHT_FS = string_t(
        "#version 330 core"

        "smooth  in  vec2    fs_texc;        // Texture coordinates from VS"
        "smooth  in  vec4    fs_color;"

        "uniform vec2        light_max;      // Max vector for the light"
        "uniform vec2        light_min;      // Max vector for the light"

        "uniform int         scr_height;     // Screen height"
        "uniform vec2        light_pos;      // Light position"
        "uniform vec3        light_col;      // Light color"
        "uniform vec3        light_att;      // Light attenuation"
        "uniform float       light_brt;      // Light brightness"

        "uniform sampler2D   geometry;       // texture with scene rendered to it"

        "smooth  out vec4    out_color;      // Output color"

        "void main()"
        "{"
        "    // Invert the fragment's y-value (I have no clue why this is necessary)"
        "    vec2 pixel      = gl_FragCoord.xy;"
        "    pixel.y         = scr_height - pixel.y;"
        "    vec2 light_vec  = light_pos  - pixel;"

        "    out_color       = vec4(0.0, 0.0, 0.0, 1.0);"

        "    if(dot(light_vec, light_max) > 0 && dot(light_vec, light_min) < 0)"
        "    {"
        "        // Calculate distance to light from fragment."
        "        float dist  = length(light_vec);"

        "        // Calculate attenuation, or light influence factor."
        "        float att   = 1.0 / ( light_att.x +"
        "                            ( light_att.y * dist) +"
        "                            ( light_att.z * dist * dist));"

        "        // Lit fragment color is the light color * attenuation * brightness."
        "        out_color   = vec4(light_col, 1.0) * vec4(att, att, att, 1.0);"
        "        out_color   = fs_color * vec4(out_color.rgb * light_brt, 1.0);"

        "        // Add regular texture color."
        "        out_color  *= texture2D(geometry, fs_texc);"
        "    }"
        "}"
    );

    /// Created from data/shaders/SpriteSheet.fs.
    static const string_t SPRITESHEET_FS = string_t(
        "#version 330 core"

        "uniform sampler2D   texture;"

        "smooth  in  vec2    fs_texc;"
        "smooth  in  vec4    fs_color;"
        "        out vec4    out_color;"

        "uniform vec2    offset;"

        "void main()"
        "{"
        "    out_color = texture2D(texture, fs_texc).rrrr * fs_color;"
        "    //+ normalize(offset));"
        "}"

    );

}
}

#endif // ZENDERER__CORE_GRAPHICS__SHADER_FILES_HPP

/** @} **/
