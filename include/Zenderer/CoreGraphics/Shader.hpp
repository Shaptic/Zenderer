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

#include <fstream>
#include <sstream>
#include <string>

#include "Zenderer/Utilities/Assert.hpp"
#include "Zenderer/Assets/Asset.hpp"
#include "OpenGL.hpp"

/// File path to engine-specific shader files.
#define ZENDERER_SHADER_PATH "Zenderer/Shaders/"

namespace zen
{
namespace gfxcore
{
    class ZEN_API CShader : public asset::CAsset
    {
    public:
        CShader(const void* const ptr);
        ~CShader();

        /**
         * Loads a shader from a file.
         *  The shader type is interpreted from the filename. Since this
         *  is a sub-class of asset::CAsset, you cannot pass any extra
         *  parameters to the object on creation.
         *  Filenames ending in `.vs` are interpreted to be vertex shaders,
         *  and those ending in `.fs` or `.ps` are interpreted to be fragment
         *  (or pixel) shaders.
         *
         * @param   filename    Filename to load from
         *
         * @return  `true`  if the file loaded and the shader compiled
         *           `false` otherwise.
         **/
        bool LoadFromFile(const string_t& filename);

        /// @overload
        bool LoadFromExisting(const CAsset* const pCopyShader);

        /// @overload
        bool LoadFromRaw(const string_t& string);

        /// Destroys the OpenGL shader object.
        bool Destroy();

        /// Returns the OpenGL shader handle (cast it to `GLuint` to use).
        const void* const GetData() const;

        /// Returns the raw OpenGL shader handle.
        GLuint GetShaderObject();

        /// Returns the shader log.
        const string_t& GetShaderLog();

    private:
        string_t m_shader_log;  ///< Log string from shader compilation.
        GLuint m_object;        ///< OpenGL shader object handle.
        GLenum m_type;          ///< Shader type (fragment, vertex, etc).
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SHADER_HPP

/** @} **/
