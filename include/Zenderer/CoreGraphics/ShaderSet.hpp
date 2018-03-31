/**
 * @file
 *  Zenderer/CoreGraphics/ShaderSet.hpp - A wrapper class for OpenGL
 *  shaders containing a set of shader objects.
 *
 * @author      george (halcyon)
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

#include <map>
#include <sstream>

#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Utilities/Assert.hpp"
#include "Shader.hpp"

namespace zen
{
namespace gfxcore
{
    using util::zLog;
    using util::LogMode;

    /// A wrapper for a usable set of shaders (an OpenGL program).
    class ZEN_API zShaderSet : public zGLSubsystem
    {
    public:
        zShaderSet(asset::zAssetManager& Assets);
        ~zShaderSet();

        /// Shallow-copy of internal shader objects.
        zShaderSet(const zShaderSet& Copy);
        zShaderSet& operator=(const zShaderSet& Copy);

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
         *  inferencing. See the docs for zShader for that.
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
         * @copydetails zen::gfxcore::zShaderSet::LoadFromFile()
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
        bool Bind() const;

        /// Removes any shader program from the rendering context.
        bool Unbind() const;

        /// Destroys the shader program.
        bool Destroy();

        /// Returns the shader program handle.
        uint16_t GetShaderObject() const;
        GLuint GetObjectHandle() const;

        /// Returns the index of a shader uniform location in VRAM.
        GLint GetUniformLocation(const string_t& name) const;

        /// Returns the index of a shader attribute location in VRAM.
        GLint GetAttributeLocation(const string_t& name) const;

        /// Returns error string.
        const string_t& GetError() const;

        /// Returns shader program link result.
        const string_t& GetLinkerLog() const;

        /// Returns program ID (unique for different shader sets).
        uint16_t GetID() const { return m_ID; }

    private:
        // To satisfy the gfxcore::zGLSubsystem requirements.
        bool Init() { return m_init = true; }

        bool DestroyFS();
        bool DestroyVS();

        /// Manages duplication of shader programs.
        static std::map<zShaderSet*, GLuint> s_shaderPrograms;
        static uint16_t s_ID;

        inline void ShowLoadError(
            const string_t& filename, const string_t& shader) const
        {
            static std::stringstream error_stream;

            error_stream.str(std::string());
            error_stream << "Failed to load " << shader << " shader from '"
                         << filename << "'.";

            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << m_Log.SetSystem("ShaderSet")
                    << error_stream.str() << zLog::endl;
        }

        inline void ShowProgramError() const
        {
            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << m_Log.SetSystem("ShaderSet")
                    << "No shader program loaded." << zLog::endl;
        }

        asset::zAssetManager&   m_AssetManager;
        util::zLog&             m_Log;

        zShader* mp_VShader;
        zShader* mp_FShader;

        string_t m_error_str;
        string_t m_link_log;

        GLuint m_program;
        uint16_t m_refcount, m_ID;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_SET_HPP

/**
 * @class zen::gfxcore::zShaderSet
 * @details
 *  This essentially a pair of shaders (vertex and fragment), in
 *  addition to some convenience methods allowing for access to
 *  the shader program's internal variables and attributes.
 **/

/** @} **/
