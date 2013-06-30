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
        CShaderSet(asset::CAssetManager& Assets);
        ~CShaderSet();

        /**
         * Loads a set of shader assets from a file.
         *  Unlike most of the other `Load*()` methods, this function
         *  *WILL* create a shader object, and thus a useable, bindable
         *  shader.
         *
         *  You can check for a linker error through GetError() and a
         *  generic log through GetLinkerLog() in order to test for warnings
         *  or other messages.
         *
         * @param   vs  Vertex shader filename
         * @param   fs  Fragment (pixel) shader filename
         *
         * @return  `true` if the shader program compiled and linked,
         *          `false` otherwise.
         *
         * @see     GetError()
         * @see     GetLinkerLog()
         **/
        bool LoadFromFile(const string_t& vs, const string_t& fs);

        /**
         * Loads a vertex shader from a file.
         *  This only loads a shader. The extension on the file is
         *  irrelevant, because the filename will be dynamically modified
         *  in order to fit the specification for dynamic shader type
         *  inferencing. See the docs for CShader for that.
         *
         *  This method doesn't create a shader program, so a call to
         *  the relevant method is required (CreateShaderObject()).
         *
         * @param   filename    Shader filename
         *
         * @return `true` if the shader object loaded from the file,
         *          `false` otherwise.
         *
         * @see     GetError()
         * @see     CreateShaderObject()
         **/
        bool LoadVertexShaderFromFile(const string_t& filename);

        /**
         * @copydoc LoadVertexShaderFromFile()
         * @brief   Loads a fragment (pixel) shader from a file.
         **/
        bool LoadFragmentShaderFromFile(const string_t& filename);

        /**
         * Loads a set of shader objects from a raw string.
         * @copydetails zen::gfxcore::CShaderSet::LoadFromFile()
         **/
        bool LoadFromStr(const string_t& vs, const string_t& fs);

        /// Loads a vertex shader from a raw string (no linking).
        bool LoadVertexShaderFromStr(const string_t& str);

        /// Loads a fragment shader from a raw string (no linking).
        bool LoadFragmentShaderFromStr(const string_t& str);

        /**
         * Links loaded shaders together into a useable program.
         *  This will take the loaded vertex and fragment shader objects
         *  and bring them together into an OpenGL shader program that
         *  can then be bound to a rendering context and used.
         *
         *  You can check for a linker error through GetError() and a
         *  generic log through GetLinkerLog() in order to test for warnings
         *  or other messages.
         *
         * @return  `true` if it linked successfully, `false` otherwise.
         *
         * @pre     A vertex and fragment shader was loaded successfully.
         *
         * @see     GetError()
         * @see     GetLinkerLog()
         **/
        bool CreateShaderObject();

        /// Binds the shader program to the rendering context for use.
        bool Bind();

        /// Removes any shader program from the rendering context.
        bool Unbind();

        /// Destroys the shader program.
        bool Destroy();

        /// Non-const because the returned handle can modify the state.
        uint16_t GetShaderObject();

        /// Returns the index of a shader uniform location in VRAM.
        short GetUniformLocation(const string_t& name);

        /// Returns the index of a shader attribute location in VRAM.
        short GetAttributeLocation(const string_t& name);

        /// Returns error string.
        const string_t& GetError() const;

        /// Returns shader program link result.
        const string_t& GetLinkerLog() const;

    private:
        inline void ShowLoadError(
            const string_t& filename, const string_t& shader)
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
        string_t m_link_log;

        GLuint m_program;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

/** @} **/
