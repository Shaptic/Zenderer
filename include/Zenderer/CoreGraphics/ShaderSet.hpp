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
    class ZEN_API CShader : public asset::CAsset
    {
    public:
        CShader(const GLenum shader_type);
        ~CShader();

        bool LoadFromFile(const string_t& filename);
        bool LoadFromExisting(const CAsset* const pCopyShader);
        bool LoadFromRaw(const string_t& string);

        /// Returns the OpenGL shader handle (cast it to `GLuint` to use).
        const void* const GetData() const
        {
            return reinterpret_cast<const void* const>(m_shader);
        }

    private:
        bool Destroy();

        GLuint m_shader;
        string_t m_vfilename, m_ffilename;
    };

    class ZEN_API CShaderSet
    {
    public:
        CShaderSet();
        ~CShaderSet();

        bool LoadFromFile(const string_t& vs, const string_t& fs);
        bool LoadVertexShaderFromFile(const string_t& filename);
        bool LoadFragmentShaderFromFile(const string_t& filename);

        bool LoadFromStr(const string_t& vs, const string_t& fs);
        bool LoadVertexShaderFromStr(const string_t& str);
        bool LoadFragmentShaderFromStr(const string_t& str);

        bool CreateShaderObject();

        bool Bind();
        bool Unbind();

        uint16_t GetShaderObject();
        short GetUniformLocation(const string_t& name);
        short GetAttributeLocation(const string_t& name);

        const string_t& GetError() const;

    private:
        CShader* mp_VShader;
        CShader* mp_FShader;

        string_t m_errorstr;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

/** @} **/
