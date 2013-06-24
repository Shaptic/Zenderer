/**
 * @file
 *  Zenderer/CoreGraphics/ShaderSet.hpp - A wrapper class for OpenGL shaders containing a set of shader objects.
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

#ifndef ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP
#define ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

#include "Zenderer/Assets/Asset.hpp"
#include "OpenGL.hpp"

namespace zen
{
namespace gfxcore
{
    class ZEN_API CShader : asset::CAsset
    {
    public:
        CShader();
        ~CShader();

        bool LoadFromFile(const string_t& vertex, const string_t& pixel);
        bool LoadFromRaw(const string_t& vertex, const string_t& pixel);

        bool Bind();
        bool Unbind();

        short GetUniformLocation(const string_t& name);
        short GetAttributeLocation(const string_t& name);

    private:
        GLint m_vshader, m_fshader;
        string_t m_vfilename, m_ffilename;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

/** @} **/
