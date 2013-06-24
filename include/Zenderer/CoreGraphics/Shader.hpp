/**
 * @file
 *  Zenderer/CoreGraphics/Shader.hpp - A wrapper around OpenGL shader objects.
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
 * @addtogroup CoreGraphics
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__SHADER_HPP
#define ZENDERER__CORE_GRAPHICS__SHADER_HPP

#include "Zenderer/Assets/Asset.hpp"
#include "OpenGL.hpp"

namespace zen
{
namespace gfxcore
{
    using util::CLog;
    using util::LogMode;

    class ZEN_API CShader : public asset::CAsset
    {
    public:
        CShader(const void* const ptr) : CAsset(ptr), m_object(0) {}
        ~CShader()
        {
            this->Destroy();
        }

        bool LoadFromFile(const string_t& filename)
        {
            if(m_loaded) this->Destroy();

            std::ifstream file(filename);
            std::stringstream source;
            std::string line;

            if(!file)
            {
                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << m_Log.SetSystem("Shader") << "Failed to open '"
                        << filename << "'." << CLog::endl;

                return false;
            }

            while(std::getline(file, line))
            {
                source << line << std::endl;
            }

            file.close();

            if(!this->LoadFromRaw(source.str())) return false;

            this->SetFilename(filename);
            return (m_loaded = true);
        }

        bool LoadFromExisting(const CAsset* const pCopyShader)
        {
            // The given parameter must must must be a CShader* instance
            // in actuality. There is no way to test for this.

            const CShader* const pCopy =
                reinterpret_cast<const CShader* const>(pCopyShader);

            ZEN_ASSERT(pCopyShader != nullptr);
            ZEN_ASSERT(pCopy != nullptr);

            m_object = reinterpret_cast<decltype(m_object)>
                (pCopyShader->GetData());

            m_type = pCopy->m_type;

            return CAsset::LoadFromExisting(pCopyShader);
        }

        bool LoadFromRaw(const string_t& string)
        {
            ZEN_ASSERT(!string.empty());

            const char* src = string.c_str();

            // Create shader object.
            GLuint shader = GL(glCreateShader(m_type));

            // Compile
            GLint error_code = GL_NO_ERROR;
            GLint length = string.length();
            GL(glShaderSource(shader, 1, &src, &length));

            ZEN_ASSERT(length == string.length());

            GL(glCompileShader(shader));
            GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &error_code));

            // We have an error
            if(error_code == GL_FALSE)
            {
                GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

                ZEN_ASSERT(length > 0);

                // Error buffer
                char* buffer = new char[length];

                GL(glGetShaderInfoLog(shader, length, &length, buffer));
                GL(glDeleteShader(shader));

                m_error_str = buffer;
                delete[] buffer;
                buffer = nullptr;

                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << m_Log.SetSystem("Shader")
                        << "Failed to compile shader: " << m_error_str
                        << CLog::endl;

                return (m_loaded = false);
            }

            m_object = shader;
            this->SetFilename("Raw shader string");
            m_error_str.clear();

            return (m_loaded = true);
        }

        /// Returns the OpenGL shader handle (cast it to `GLuint` to use).
        const void* const GetData() const
        {
            return reinterpret_cast<const void* const>(m_object);
        }

        inline GLuint GetShaderObject()
        {
            return m_object;
        }

        bool Destroy()
        {
            if(m_object > 0)
            {
                GL(glDeleteShader(m_object));

                m_filename = "";
                m_object = m_filename_hash = 0;
            }

            return !(m_loaded = false);
        }

    private:
        GLuint m_object;        ///< OpenGL shader object handle
        GLenum m_type;          ///< Shader type (fragment, vertex, etc.)
    };
}   // namespace gfxcore
}   // namespace zen


#endif // ZENDERER__CORE_GRAPHICS__SHADER_HPP

/** @} **/
