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
        "#version 330 core\n"

        "smooth  in  vec2    fs_texc;    // Texture coordinates from VS\n"
        "smooth  in  vec4    fs_color;\n"

        "uniform vec3        light_col;  // Light color\n"
        "uniform float       light_brt;  // Light brightness\n"

        "uniform sampler2D   geometry;   // texture with scene rendered to it\n"

        "smooth  out vec4    out_color;  // Output color\n"

        "void main()\n"
        "{\n"
        "    out_color   = light_brt * vec4(light_col, 1.0);\n"
        "    out_color  *= fs_color * texture2D(geometry, fs_texc);\n"
        "}\n"
    );

    /// Created from data/shaders/Default.fs.
    static const string_t DEFAULT_FS = string_t(
        "#version 330 core\n"

        "uniform sampler2D   texture;\n"

        "smooth  in  vec2 fs_texc;\n"
        "smooth  in  vec4 fs_color;\n"
        "        out vec4 out_color;\n"

        "void main()\n"
        "{\n"
        "    out_color = texture2D(texture, fs_texc) * fs_color;\n"
        "}\n"
    );

    /// Created from data/shaders/Default.vs.
    static const string_t DEFAULT_VS = string_t(
        "#version 330 core\n"

        "layout(location=0) in vec3  vs_vertex;\n"
        "layout(location=1) in vec2  vs_texc;\n"
        "layout(location=2) in vec4  vs_color;\n"

        "uniform mat4    proj;\n"
        "uniform mat4    mv;\n"

        "smooth  out vec3 fs_vertex;\n"
        "smooth  out vec4 fs_color;\n"
        "smooth  out vec2 fs_texc;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = proj * mv * vec4(vs_vertex, 1.0);\n"

        "    fs_color    = vs_color;\n"
        "    fs_texc     = vs_texc;\n"
        "    fs_vertex   = vs_vertex;\n"
        "}\n"
    );

    /// Created from data/shaders/Fade.fs.
    static const string_t FADE_FS = string_t(
        "#version 330 core\n"

        "uniform float       alpha;\n"
        "uniform sampler2D   texture;\n"
        "uniform vec2        tc_offset;\n"

        "smooth  in  vec2 fs_texc;\n"
        "smooth  in  vec4 fs_color;\n"
        "        out vec4 out_color;\n"

        "void main()\n"
        "{\n"
        "    vec4 final = texture2D(texture, fs_texc + tc_offset) * fs_color;\n"
        "    out_color  = vec4(alpha * final.rgb, final.a);\n"
        "}\n"
    );

    /// Created from data/shaders/GaussianBlur.vs.
    static const string_t GAUSSIANBLUR_VS = string_t(
        "#version 330 core\n"

        "layout(location=0) in vec2  vs_vertex;\n"
        "layout(location=1) in vec2  vs_texc;\n"
        "layout(location=2) in vec4  vs_color;\n"

        "uniform mat4 proj;\n"
        "uniform mat4 mv;\n"

        "smooth  out vec2 fs_vertex;\n"
        "smooth  out vec4 fs_color;\n"
        "smooth  out vec2 fs_texc;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = proj * mv * vec4(vs_vertex, 1.0, 1.0);\n"

        "    fs_color    = vs_color;\n"
        "    fs_texc     = vs_texc;\n"
        "    fs_vertex   = sign(vs_vertex.xy) * 0.5 + 0.5;\n"
        "}\n"
    );

    /// Created from data/shaders/GaussianBlurH.fs.
    static const string_t GAUSSIANBLURH_FS = string_t(
        "#version 330 core\n"

        "smooth in vec2      fs_texc;\n"
        "smooth in vec4      fs_color;\n"
        "smooth out vec4     final_color;\n"

        "uniform sampler2D   texture;\n"
        "uniform float       radius;\n"

        "// Performs a vertical blur by sampling 9 pixels.\n"
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0.0);\n"

        "    // blur in y (horizontal)\n"
        "    // take nine samples, with the distance 'radius' between them\n"
        "    sum += texture2D(texture, vec2(fs_texc.x - 4.0*radius, fs_texc.y)) * 0.05;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x - 3.0*radius, fs_texc.y)) * 0.09;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x - 2.0*radius, fs_texc.y)) * 0.12;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x - radius, fs_texc.y))     * 0.15;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y))              * 0.16;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x + radius, fs_texc.y))     * 0.15;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x + 2.0*radius, fs_texc.y)) * 0.12;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x + 3.0*radius, fs_texc.y)) * 0.09;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x + 4.0*radius, fs_texc.y)) * 0.05;\n"

        "    final_color = sum * fs_color;\n"
        "}\n"
    );

    /// Created from data/shaders/GaussianBlurV.fs.
    static const string_t GAUSSIANBLURV_FS = string_t(
        "#version 330 core\n"

        "smooth in vec2      fs_texc;\n"
        "smooth in vec4      fs_color;\n"
        "smooth out vec4     final_color;\n"

        "uniform sampler2D   texture;\n"
        "uniform float       radius;\n"

        "// Performs a horizontal blur by sampling 9 pixels.\n"
        "void main(void)\n"
        "{\n"
        "    vec4 sum = vec4(0.0);\n"

        "    // blur in y (vertical)\n"
        "    // take nine samples, with the distance 'radius' between them\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 4.0*radius))  * 0.05;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 3.0*radius))  * 0.09;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - 2.0*radius))  * 0.12;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y - radius))      * 0.15;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y))               * 0.16;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + radius))      * 0.15;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 2.0*radius))  * 0.12;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 3.0*radius))  * 0.09;\n"
        "    sum += texture2D(texture, vec2(fs_texc.x, fs_texc.y + 4.0*radius))  * 0.05;\n"

        "    final_color = sum * fs_color;\n"
        "}\n"
    );

    /// Created from data/shaders/Grayscale.fs.
    static const string_t GRAYSCALE_FS = string_t(
        "#version 330 core\n"

        "uniform sampler2D   texture;\n"

        "smooth  in  vec2    fs_texc;\n"
        "smooth  in  vec4    fs_color;\n"
        "        out vec4    out_color;\n"

        "void main()\n"
        "{\n"
        "    vec4 color = texture2D(texture, fs_texc);\n"
        "    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n"
        "    out_color  = vec4(color.rgb * gray, color.a);\n"
        "}\n"
    );

    /// Created from data/shaders/PointLight.fs.
    static const string_t POINTLIGHT_FS = string_t(
        "#version 330 core\n"

        "smooth  in  vec2    fs_texc;        // Texture coordinates from VS\n"
        "smooth  in  vec4    fs_color;\n"

        "uniform int         scr_height;     // Screen height\n"
        "uniform vec2        light_pos;      // Light position\n"
        "uniform vec3        light_col;      // Light color\n"
        "uniform vec3        light_att;      // Light attenuation\n"
        "uniform float       light_brt;      // Light brightness\n"

        "uniform sampler2D   geometry;       // texture with scene rendered to it\n"

        "smooth  out vec4    out_color;      // Output color\n"

        "void main()\n"
        "{\n"
        "    // Invert the fragment's y-value (I have no clue why this is necessary)\n"
        "    vec2 pixel      = gl_FragCoord.xy;\n"
        "    pixel.y         = scr_height - pixel.y;\n"

        "    // Calculate distance to light from fragment.\n"
        "    vec2 light_vec  = light_pos  - pixel;\n"
        "    float dist      = length(light_vec);\n"

        "    // Calculate attenuation, or light influence factor.\n"
        "    float att       = 1.0 / ( light_att.x +\n"
        "                            ( light_att.y * dist) +\n"
        "                            ( light_att.z * dist * dist));\n"

        "    // Final fragment color is the light color * attenuation * brightness.\n"
        "    out_color       = texture2D(geometry, fs_texc);\n"
        "    out_color      *= vec4(light_col, 1.0) * vec4(att, att, att, 1.0);\n"
        "    out_color       = fs_color * vec4(out_color.rgb * light_brt, 1.0);\n"
        "}\n"
    );

    /// Created from data/shaders/Ripple.fs.
    static const string_t RIPPLE_FS = string_t(
        "#version 330 core\n"

        "uniform float       rate;\n"
        "uniform float       time;   // time in seconds\n"
        "uniform sampler2D   tex;    // scene buffer\n"

        "smooth  in  vec2 fs_texc;\n"
        "smooth  in  vec4 fs_color;\n"
        "        out vec4 out_color;\n"

        "void main(void)\n"
        "{\n"
        "  vec2 pos  = -1.0 + 2.0 * fs_texc;//gl_FragCoord.xy / res;\n"
        "  float len = length(pos * max(0.2, time * rate));\n"
        "  vec2 uv   = fs_texc + (pos / len) * cos(len * 12.0 - time * 4.0) * 0.03;\n"

        "  out_color = texture2D(tex, uv) * fs_color;\n"
        "}\n"
    );

    /// Created from data/shaders/SpotLight.fs.
    static const string_t SPOTLIGHT_FS = string_t(
        "#version 330 core\n"

        "smooth  in  vec2    fs_texc;        // Texture coordinates from VS\n"
        "smooth  in  vec4    fs_color;\n"

        "uniform vec2        light_max;      // Max vector for the light\n"
        "uniform vec2        light_min;      // Max vector for the light\n"

        "uniform int         scr_height;     // Screen height\n"
        "uniform vec2        light_pos;      // Light position\n"
        "uniform vec3        light_col;      // Light color\n"
        "uniform vec3        light_att;      // Light attenuation\n"
        "uniform float       light_brt;      // Light brightness\n"

        "uniform sampler2D   geometry;       // texture with scene rendered to it\n"

        "smooth  out vec4    out_color;      // Output color\n"

        "void main()\n"
        "{\n"
        "    // Invert the fragment's y-value (I have no clue why this is necessary)\n"
        "    vec2 pixel      = gl_FragCoord.xy;\n"
        "    pixel.y         = scr_height - pixel.y;\n"
        "    vec2 light_vec  = light_pos  - pixel;\n"

        "    out_color       = vec4(0.0, 0.0, 0.0, 1.0);\n"

        "    if(dot(light_vec, light_max) > 0 && dot(light_vec, light_min) < 0)\n"
        "    {\n"
        "        // Calculate distance to light from fragment.\n"
        "        float dist  = length(light_vec);\n"

        "        // Calculate attenuation, or light influence factor.\n"
        "        float att   = 1.0 / ( light_att.x +\n"
        "                            ( light_att.y * dist) +\n"
        "                            ( light_att.z * dist * dist));\n"

        "        // Lit fragment color is the light color * attenuation * brightness.\n"
        "        out_color   = vec4(light_col, 1.0) * vec4(att, att, att, 1.0);\n"
        "        out_color   = fs_color * vec4(out_color.rgb * light_brt, 1.0);\n"

        "        // Add regular texture color.\n"
        "        out_color  *= texture2D(geometry, fs_texc);\n"
        "    }\n"
        "}\n"
    );

    /// Created from data/shaders/SpriteSheet.fs.
    static const string_t SPRITESHEET_FS = string_t(
        "#version 330 core\n"

        "uniform sampler2D   texture;\n"
        "uniform float       tc_offset;\n"
        "uniform vec2        tc_start;\n"

        "smooth  in  vec2 fs_texc;\n"
        "smooth  in  vec4 fs_color;\n"
        "        out vec4 out_color;\n"

        "void main()\n"
        "{\n"
        "    vec2 coord= vec2(tc_start.s + (fs_texc.s * tc_offset), tc_start.t + fs_texc.t);\n"
        "    out_color = texture2D(texture, coord) * fs_color;\n"
        "}\n"
    );

    /// Created from data/shaders/zFont.fs.
    static const string_t ZFONT_FS = string_t(
        "#version 330 core\n"

        "uniform sampler2D   texture;\n"

        "smooth  in  vec2    fs_texc;\n"
        "smooth  in  vec4    fs_color;\n"
        "        out vec4    out_color;\n"

        "uniform vec2    offset;\n"

        "void main()\n"
        "{\n"
        "    out_color = texture2D(texture, fs_texc).rrrr * fs_color;\n"
        "}\n"

    );

}
}

#endif // ZENDERER__CORE_GRAPHICS__SHADER_FILES_HPP

/** @} **/
