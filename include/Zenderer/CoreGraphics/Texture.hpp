/**
 * @file
 *  Zenderer/CoreGraphics/Texture.hpp - A low-level texture asset wrapper
 *  around OpenGL textures.
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

#ifndef ZENDERER__CORE_GRAPHICS__TEXTURE_HPP
#define ZENDERER__CORE_GRAPHICS__TEXTURE_HPP

#include <sstream>

#include "stb_image.h"

#include "Zenderer/Assets/Asset.hpp"
#include "Zenderer/Utilities/Log.hpp"
#include "OpenGL.hpp"

namespace zen
{
namespace gfxcore
{
    /// Low-level texture asset abstraction over OpenGL texture handles.
    class ZEN_API CTexture : public asset::CAsset
    {
    public:
        ~CTexture();

        bool LoadFromFile(const string_t& filename);
        bool LoadFromExisting(const CAsset* const pCopy);
        bool LoadFromExisting(const GLuint handle);
        bool LoadFromRaw(const GLenum iformat, const GLenum format,
                         const uint16_t w, const uint16_t h,
                         const unsigned char* data);

        const void* const GetData() const;

        bool Bind() const;
        bool Unbind() const;

        uint16_t GetWidth()     const { return m_width; }
        uint16_t GetHeight()    const { return m_height; }

        static CTexture& GetDefaultTexture();

        friend class asset::CAssetManager;

    private:
        CTexture(const void* const owner = nullptr);
        bool Destroy();

        static CTexture s_DefaultTexture;

        GLuint m_texture;
        GLuint m_width, m_height;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__TEXTURE_HPP

/**
 * @class zen::gfxcore::CTexture
 *
 * @details
 *  @a Zenderer uses `stb_image.c`, a single-file light-weight C library, 
 *  to load a variety of image formats. It has only been tested internally
 *  with 24-bit and 32-bit `.png` files. It will internally attempt force
 *  a conversion to a 32-bit RGBA format. If this doesn't succeed, it will
 *  obviously be apparent as soon as the texture is rendered.
 **/

/** @} **/
