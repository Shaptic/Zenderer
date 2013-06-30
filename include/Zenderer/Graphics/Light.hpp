/**
 * @file
 *  Zenderer/Graphics/Light.hpp - An instance of lighting in the game scene.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.0
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

#ifndef ZENDERER__GRAPHICS__LIGHT_HPP
#define ZENDERER__GRAPHICS__LIGHT_HPP

#include "Zenderer/Math/Matrix.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/CoreGraphics/ShaderSet.hpp"

namespace zen
{
namespace gfx
{
    /// Lighting types.
    ZEN_API enum class LightType : int16_t
    {
        ZEN_NO_LIGHT = -1,  ///< An invalid light type
        ZEN_AMBIENT,        ///< Uniform ambient light
        ZEN_SPOTLIGHT,      ///< Angled spot light
        ZEN_POINT           ///< Omnidirectional point light
    };

    /**
     * Represents a light object acting on a graphical scene.
     *  Each lighting type has its own special variables and parameters 
     *  that can be modified and accessed. See the raw shader files to learn
     *  the specific parameter names.
     **/
    class ZEN_API CLight
    {
    public:
        /**
         * Constructs a light instance.
         *  The window height is required for the shader to perform some weird
         *  offset calculations that would otherwise give bad results in terms
         *  of light positioning. It defaults to 800px.
         *
         * @param   m_Assets    The asset manager to make the effect from
         * @param   Type        The light type you want
         * @param   window_h    The height of the OpenGL context 
         **/
        CLight(asset::CAssetManager& m_Assets, const LightType& Type,
               const uint16_t window_h = 800) :
            m_type(Type), m_Shader(m_Assets), m_height(window_h) {}

        ~CLight() {}

        bool Init()
        {
            bool state = false;

            state = m_Shader.LoadVertexShaderFromFile(
                ZENDERER_SHADER_PATH"Default.vs");

            switch(m_type)
            {
            case LightType::ZEN_AMBIENT:
                state = state && m_Shader.LoadFragmentShaderFromFile(
                    ZENDERER_SHADER_PATH"AmbientLight.fs");
                break;

            case LightType::ZEN_POINT:
                state = state && m_Shader.LoadFragmentShaderFromFile(
                    ZENDERER_SHADER_PATH"PointLight.fs");
                break;

            case LightType::ZEN_SPOTLIGHT:
                state = state && m_Shader.LoadFragmentShaderFromFile(
                    ZENDERER_SHADER_PATH"SpotLight.fs");
                break;

            default: break;
            }

            state = state && m_Shader.CreateShaderObject();

            if(state)
            {
                m_loccol = m_Shader.GetUniformLocation("light_col");
                m_locbrt = m_Shader.GetUniformLocation("light_brt");
                m_locpos = m_Shader.GetUniformLocation("light_pos");
                m_loctmx = m_Shader.GetUniformLocation("light_max");
                m_loctmn = m_Shader.GetUniformLocation("light_min");

                GLint mvloc     = m_Shader.GetUniformLocation("mv"),
                      projloc   = m_Shader.GetUniformLocation("proj"),
                      scrloc    = m_Shader.GetUniformLocation("scr_height");

                if(m_loccol == -1   || m_locbrt == -1 || mvloc == -1 ||
                   projloc == -1    || scrloc == -1)
                {
                    return false;
                }
                else
                {
                    // Set up the values we only need a single time.
                    if(!m_Shader.Bind()) return false;

                    GL(glUniformMatrix4fv(mvloc, 1, GL_TRUE, 
                            math::matrix4x4_t
                                ::GetIdentityMatrix().GetPointer()));

                    GL(glUniformMatrix4fv(projloc, 1, GL_TRUE, 
                            gfxcore::CRenderer
                                   ::GetProjectionMatrix().GetPointer()));

                    GL(glUniform1i(scrloc, m_height));

                    return m_Shader.Unbind();
                }
            }

            return state;
        }

        bool Enable()
        {
            return m_Shader.Bind();
        }

        bool Disable()
        {
            return m_Shader.Unbind();
        }

    private:
        static math::vector_t   s_DefaultColor, s_DefaultAttenuation,
                                s_DefaultPosition;
        static real_t           s_DefaultBrightness, 
                                s_DefaultMaxAngle, s_DefaultMinAngle;

        // We don't use the high-level effect to get more control
        // and minimize uniform parameter lookups.
        gfxcore::CShaderSet m_Shader;

        LightType m_type;

        // Uniform locations
        GLint m_loccol,     // Color
            m_locbrt,     // Brightness
            m_locpos,     // Position
            m_loctmx,     // Minimum angle
            m_loctmn;     // Maximum angle

        // OpenGL context height (in pixels).
        uint16_t m_height;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__LIGHT_HPP

/** @} **/
