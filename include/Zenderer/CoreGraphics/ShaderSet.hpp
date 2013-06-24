/**
 * @file
 *  Zenderer/CoreGraphics/ShaderSet.hpp - A wrapper class for OpenGL
 *  shaders containing a set of shader objects.
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

#ifndef ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP
#define ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

#include <sstream>

#include "Zenderer/Assets/AssetManager.hpp"
#include "Shader.hpp"

namespace zen
{
namespace gfxcore
{
    using util::CLog;
    using util::LogMode;

    class ZEN_API CShaderSet
    {
    public:
        CShaderSet(asset::CAssetManager& Assets) :
            m_AssetManager(Assets), m_Log(CLog::GetEngineLog()),
            mp_FShader(nullptr), mp_VShader(nullptr), m_program(0),
            m_error_str("")
        {

        }

        ~CShaderSet()
        {
            this->Destroy();
        }

        /**
         * Loads a set of shader assets from a file and creates an object.
         *  Unlike the other `Load*()` methods, this function *WILL* create
         *  a shader object.
         *
         * @param   string_t    Vertex shader filename
         * @param   string_t    Fragment (pixel) shader filename
         *
         * @return  `true` if the shader program compiled and linked,
         *          `false` otherwise.
         *
         * @see     GetError()
         **/
        bool LoadFromFile(const string_t& vs, const string_t& fs)
        {
            // Kill any existing shader programs.
            this->Destroy();

            mp_FShader = m_AssetManager.Create<CShader>(vs);
            mp_VShader = m_AssetManager.Create<CShader>(fs);

            if(mp_FShader == nullptr)
            {
                this->ShowLoadError(fs, "fragment");
                return false;
            }

            if(mp_VShader == nullptr)
            {
                this->ShowLoadError(vs, "vertex");
                return false;
            }

            return this->CreateShaderObject();
        }

        bool LoadVertexShaderFromFile(const string_t& filename)
        {
            this->Destroy();
            
            mp_VShader = m_AssetManager.Create<CShader>(filename);
            if(mp_VShader == nullptr)
            {
                this->ShowLoadError(filename, "vertex");
                return false;
            }
            
            return true;
        }
        
        bool LoadFragmentShaderFromFile(const string_t& filename)
        {
            
        }

        bool LoadFromStr(const string_t& vs, const string_t& fs);
        bool LoadVertexShaderFromStr(const string_t& str);
        bool LoadFragmentShaderFromStr(const string_t& str);

        bool CreateShaderObject()
        {
            if(mp_FShader == nullptr || mp_VShader == nullptr)
            {
                m_error_str = "No shader objects loaded.";

                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << m_Log.SetSystem("ShaderSet")
                        << m_error_str << CLog::endl;

                return false;
            }

            // Create shader program and attach shaders.
            m_program = glCreateProgram();
            glAttachShader(m_program, mp_VShader->GetShaderObject());
            glAttachShader(m_program, mp_FShader->GetShaderObject());

            // Link the compiled shader objects to the program.
            GLint err = GL_NO_ERROR;
            glLinkProgram(m_program);
            glGetProgramiv(m_program, GL_LINK_STATUS, &err);

            // Link failed?
            if(err == GL_FALSE)
            {
                int length  = 0;

                // Get log length to make an appropriate buffer.
                glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);

                // Delete old log.
                m_error_str.clear();

                // Get log.
                char* buffer = new char[length];
                glGetProgramInfoLog(m_program, length, &length, buffer);
                glDeleteProgram(m_program);

                m_error_str = buffer;
                delete[] buffer;

                // Show log.
                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << m_Log.SetSystem("ShaderSet")
                        << "Failed to link shader objects to program: "
                        << m_error_str << "." << CLog::endl;

                this->Destroy();

                return false;
            }

            return true;
        }

        bool Bind()
        {
            if(m_program == 0)
            {
                this->ShowProgramError();
                return false;
            }

            GL(glUseProgram(m_program));
            return true;
        }

        bool Unbind()
        {
            if(m_program == 0)
            {
                this->ShowProgramError();
                return false;
            }

            GL(glUseProgram(0));
            return true;
        }

        /// Non-const because the returned handle can modify the state.
        uint16_t GetShaderObject()
        {
            return m_program;
        }

        short GetUniformLocation(const string_t& name)
        {
            if(m_program == 0)
            {
                this->ShowProgramError();
                return -1;
            }

            GLint loc = -1;
            GL(loc = glGetUniformLocation(m_program, name.c_str()));
            return loc;
        }

        short GetAttributeLocation(const string_t& name)
        {
            if(m_program == 0)
            {
                this->ShowProgramError();
                return -1;
            }

            GLint loc = -1;
            GL(loc = glGetAttribLocation(m_program, name.c_str()));
            return loc;
        }

        const string_t& GetError() const
        {
            return m_error_str;
        }

    private:
        void Destroy()
        {
            m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                    << m_Log.SetSystem("ShaderSet")
                    << "Destorying shader set." << CLog::endl;

            if(mp_FShader != nullptr)
            {
                m_AssetManager.Delete(mp_FShader);
                mp_FShader = nullptr;
            }

            if(mp_VShader != nullptr)
            {
                m_AssetManager.Delete(mp_VShader);
                mp_VShader = nullptr;
            }

            if(m_program > 0)
            {
                GL(glDeleteProgram(m_program));
                m_program = 0;
            }

            m_error_str = "";
        }
        
        inline void ShowLoadError(const string_t& filename, const string_t& shader)
        {
            static std::stringstream error_stream;
            
            error_stream.str(std::string());
            error_stream << "Failed to load " << shader << " shader from '"
                         << filename << "'.";
                         
            m_error_str = error_stream.str();
            
            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << m_Log.SetSystem("ShaderSet")
                    << error_stream.str() << CLog::endl;
        }
        
        inline void ShowProgramError()
        {
            m_error_str = "No shader program loaded.";
            
            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << m_Log.SetSystem("ShaderSet")
                    << m_error_str << CLog::endl;
        }

        asset::CAssetManager&   m_AssetManager;
        util::CLog&             m_Log;

        CShader* mp_VShader;
        CShader* mp_FShader;

        string_t m_error_str;

        GLuint m_program;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

/** @} **/
