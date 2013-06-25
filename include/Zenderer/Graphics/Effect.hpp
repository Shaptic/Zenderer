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

namespace zen
{
namespace gfx
{
    enum class EffectType : int16_t
    {
        NO_EFFECT = -1,
        GAUSSIAN_BLUR_H,
        GAUSSIAN_BLUR_V,
        GRAYSCALE,
        FADE,
        RIPPLE,
        ZEN_EFFECT_COUNT
    };

    /**
     * A high-level shader wrapper. 
     *  This class is very similar to the gfxcore::CShaderSet class,
     *  but creates a more user-friendly interface
     **/
    class ZEN_API CEffect : public gfxcore::CGLSubsystem
    {
    public:
        CEffect(asset::CAssetManager& Assets, const EffectType Type);
        ~CEffect();
        
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
         * @param   string_t    Parameter name
         * @param   T*          Array of values of type T (or single)
         * @param   size_t      Amount of values passed (optional=`1`)
         *
         * @return  `true` if the parameter was found for the effect
         *          `false` if the effect was not loaded.
         **/
        bool SetParameter(const string_t& name,
                          const real_t* pValues,
                          const size_t count);

        /// @overload
        bool SetParameter(const string_t& name,
                          const int* pValues,
                          const size_t count);
        
        /**
         * Modifies a matrix parameter in the effect.
         *  The behavior of this method is identical to that of SetParameter(),
         *  but it modifies a matrix instead of a primitive type
         *  (`int`, `float`, ...)
         *
         * @param   string_t            Name of matrix parameter
         * @param   math::matrix4x4_t   Matrix value to send to effect
         * 
         * @return  `true` if the parameter was found and set, 
         *          `false` otherwise.
         *
         * @overload
         **/
        bool SetParameter(const string_t& name,
                          const math::matrix4x4_t& Matrix);
        
        inline bool Enable();   ///< A more user-friendly alias for binding.
        inline bool Disable();  ///< A more user-friendly alias for unbinding.
        
        inline const string_t& GetError() const;
        inline void SetType(const EffectType Type);
        
    private:
        inline bool Bind();
        inline bool Unbind();

        util::CLog& m_Log;

        gfxcore::CShaderSet m_Shader;
        EffectType m_type;
    };
    
    #include "Effect.inl"
}   // namespace gfx
}   // namespace zen

#endif  // ZENDERER__GRAPHICS__EFFECT_HPP
 