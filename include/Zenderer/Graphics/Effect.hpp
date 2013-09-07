/**
 * @file
 *  Zenderer/Graphics/Effect.hpp - A high-level shader wrapper intended to
 *  be used by the user, abstracting shader variable set up.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.1
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
 * @addtogroup Graphics
 * @{
 **/

#ifndef ZENDERER__GRAPHICS__EFFECT_HPP
#define ZENDERER__GRAPHICS__EFFECT_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Math/Matrix.hpp"
#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/ShaderSet.hpp"
#include "Zenderer/CoreGraphics/ShaderFiles.hpp"

namespace zen
{
namespace gfx
{
    /**
     * Names a distribution of effect types.
     *  See the table in the zEffect documentation for appropriate
     *  parameters for each of the effects.
     *
     * @note    `CUSTOM_EFFECT` is only used by the material wrapper.
     * @todo    Fix table border problem in detailed docs.
     **/
    enum class ZEN_API EffectType : int16_t
    {
        NO_EFFECT = -1,     ///< No effect, default shaders
        CUSTOM_EFFECT,      ///< A custom effect loaded from a file
        GAUSSIAN_BLUR_H,    ///< A horizontal Gaussian blur effect
        GAUSSIAN_BLUR_V,    ///< A vertical Gaussian blur effect
        GRAYSCALE,          ///< Makes the target grayscale
        SPRITESHEET,        ///< Render texture atlases using offsets
        FADE,               ///< Fades the target in/out
        RIPPLE,             ///< Creates a centered ripple effect
        ZEN_EFFECT_COUNT    ///< Total amount of effects
    };

    /// A high-level shader wrapper.
    class ZEN_API zEffect : public gfxcore::zGLSubsystem
    {
    public:
        zEffect(const EffectType Type, asset::zAssetManager& Assets);
        ~zEffect();

        /// Shallow-copy the shader references and other metadata.
        zEffect(const zEffect& Copy) : m_Log(Copy.m_Log),
            m_Shader(Copy.m_Shader), m_type(Copy.m_type)
        {
            m_init = true;
        }

        zEffect& operator=(const zEffect& Copy)
        {
            m_Shader= Copy.m_Shader;
            m_type  = Copy.m_type;
            m_init  = Copy.m_init;
            return (*this);
        }

        /// @todo   Use Gaussian Blur vertex shaders.
        /// @todo   Add logging for shader type in Release builds.
        bool Init();
        bool Destroy();

        /**
         * Modifies the effect based on a certain parameter.
         *  The parameter name must directly correspond to the
         *  uniform variable name found in the respective effect
         *  shader file.
         *
         *  There are multiple overloads to support a variety of
         *  parameter types and value combinations.
         *
         *  Regardless of whether or not `real_t` is defined as a
         *  `float` or a `double`, GLSL only deals with
         *  single-precision uniform values.
         *
         * @param   name    Parameter name
         * @param   pValues Array of values of type T (or single)
         * @param   count   Amount of values passed (optional=`1`)
         *
         * @return  `true` if the parameter was found for the effect
         *          `false` if the effect was not loaded.
         **/
        bool SetParameter(const string_t& name,
                          const real_t* pValues,
                          const size_t count = 1);

        /// @overload
        bool SetParameter(const string_t& name,
                          const int* pValues,
                          const size_t count = 1);

        /**
         * Modifies a matrix parameter in the effect.
         *  The behavior of this method is identical to that of SetParameter(),
         *  but it modifies a matrix instead of a primitive type
         *  (`int`, `float`, ...)
         *
         * @param   name    Name of matrix parameter
         * @param   Matrix  Matrix value to send to effect
         *
         * @return  `true` if the parameter was found and set,
         *          `false` otherwise.
         *
         * @overload
         **/
        bool SetParameter(const string_t& name,
                          const math::matrix4x4_t& Matrix);

        inline bool Enable() const;     ///< A more user-friendly alias for binding.
        inline bool Disable() const;    ///< A more user-friendly alias for unbinding.

        inline GLuint GetObjectHandle() const;
        inline const string_t& GetError() const;
        inline EffectType GetType() const { return m_type; }
        inline uint16_t GetID() const { return m_Shader.GetID(); }
        inline void SetType(const EffectType Type);
        friend class ZEN_API zMaterial;

    private:
        // For the material to be able to load custom shaders.
        bool LoadCustomEffect(const string_t& vs, const string_t& fs);

        inline bool Bind() const;
        inline bool Unbind() const;

        util::zLog& m_Log;

        gfxcore::zShaderSet m_Shader;
        EffectType m_type;
    };

    #include "Effect.inl"
}   // namespace gfx
}   // namespace zen

#endif  // ZENDERER__GRAPHICS__EFFECT_HPP

/**
 * @class zen::gfx::zEffect
 * @details
 *  This class is very similar to the gfxcore::zShaderSet class,
 *  but creates a more user-friendly interface
 *
 * @htmlonly
 *  <style>
 *  table.custom,
 *  table.custom td {
 *    border: 1px solid;
 *    border-collapse: collapse;
 *    padding: 5px;
 *  }
 *
 *  table.custom table,
 *  table.custom td td {
 *    border: none;
 *    padding-right: 15px;
 *  }
 *
 *  table.custom span {
 *    font-family: monospace;
 *  }
 *  </style>
 *
 *  <table class="custom">
 *  <tr>
 *    <th>Enumeration</th>
 *    <th>Shaders</th>
 *    <th>Description</th>
 *    <th>Parameters</th>
 *  </tr>
 *  <tr>
 *    <td><pre>NO_EFFECT</pre></td>
 *    <td><pre>Default.vs<br/>Default.fs</pre></td>
 *    <td>Default rendering shader</td>
 *    <td></td>
 *  </tr>
 *  <tr>
 *    <td><pre>GAUSSIAN_BLUR_H</pre></td>
 *    <td><pre>GaussianBlur.vs<br/>GaussianBlurH.fs</pre></td>
 *    <td>
 *      Vertical Gaussian blur effect<br/>This should be used in a multi-pass
 *      combination with<br/><span>GAUSSIAN_BLUR_V</span> for optimal output
 *    </td>
 *    <td>
 *      <table>
 *      <tr>
 *        <td><span>radius</span></td>
 *        <td>The blur radius,<br/>between [0.0, 1.0]</td>
 *      </tr>
 *      </table>
 *    </td>
 *  </tr>
 *  <tr>
 *    <td><pre>GAUSSIAN_BLUR_H</pre></td>
 *    <td><pre>GaussianBlur.vs<br/>GaussianBlurV.fs</pre></td>
 *    <td>
 *      Horizontal Gaussian blur effect<br/>This should be used in a
 *      multi-pass with<br/><span>GAUSSIAN_BLUR_H</span> for optimal output
 *    </td>
 *    <td>
 *      <table>
 *      <tr>
 *        <td><span>radius</span></td>
 *        <td>The blur radius,<br/>between [0.0, 1.0]</td>
 *      </tr>
 *      </table>
 *    </td>
 *  </tr>
 *  <tr>
 *    <td><pre>GRAYSCALE</pre></td>
 *    <td><pre>Default.vs<br/>Grayscale.fs</pre></td>
 *    <td>
 *      Converts all colors to grayscale using NTSC weights<br/>
 *      They are (0.299, 0.587, 0.114)
 *    </td>
 *    <td>n/a</td>
 *  </tr>
 *  <tr>
 *    <td><pre>SPRITESHEET</pre></td>
 *    <td><pre>Default.vs<br/>SpriteSheet.fs</pre></td>
 *    <td>
 *      Renders a texture using an offset, thus only rendering a
 *      portion of it. Useful for sprite sheets and other texture atlases.
 *      The size of the texture rendered will match the size of your sprite,
 *      and tiling / stretching is impossible.
 *    </td>
 *    <td>
 *      <table>
 *      <tr>
 *        <td><span>offset</span></td>
 *        <td>The offset, in pixels, with which to render the texture.</td>
 *      </tr>
 *      </table>
 *    </td>
 *  </tr>
 *  <tr>
 *    <td><pre>FADE</pre></td>
 *    <td><pre>Default.vs<br/>Fade.fs</pre></td>
 *    <td>
 *      Applies a weight to all of the colors to darken or lighten,<br/>
 *      simulating a fading effect.
 *    </td>
 *    <td>
 *      <table>
 *      <tr>
 *        <td><span>alpha</span></td>
 *        <td>The fade level. 0.0 is fully faded to black.</td>
 *      </tr>
 *      </table>
 *    </td>
 *  </tr>
 *  <tr>
 *    <td><pre>RIPPLE</pre></td>
 *    <td><pre>Default.vs<br/>Ripple.fs</pre></td>
 *    <td>Simulates a water ripple effect on a texture.</td>
 *    <td>
 *      <table>
 *      <tr>
 *        <td><span>time</span></td>
 *        <td>The total time elapsed</td>
 *      </tr>
 *      <tr>
 *        <td><span>rate </span></td>
 *        <td>The rate at which to ripple</td>
 *      </tr>
 *      </table>
 *    </td>
 *  </tr>
 *  </table>
 * @endhtmlonly
 *
 **/

/** @} **/
