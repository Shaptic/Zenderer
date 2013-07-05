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
        ~CTexture() { this->Destroy(); }

        bool LoadFromFile(const string_t& filename)
        {
            if(m_loaded) this->Destroy();

            int32_t w, h, comp;
            unsigned char* raw = stbi_load(filename.c_str(), &w, &h, &comp,
                4 /* force 32-bit image (4 x 8-bit) */);
            if(raw == nullptr) return false;

            ZEN_ASSERT(w > 0 && h > 0);

            stbi_flip_y(w, h, 4, raw);
            if(comp != 4 && comp != 3)
            {
                m_error_str = "Invalid number of components. "
                    "Textures must be 24-bit (RGB) or 32-bit (RGBA) files.";

                m_Log << m_Log.SetMode(util::LogMode::ZEN_ERROR)
                      << m_Log.SetSystem("Texture") << m_error_str
                      << util::CLog::endl;

                return (m_loaded = false);
            }

            bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA, w, h, raw);

            stbi_image_free(raw);
            this->SetFilename(filename);
            return (m_loaded = true);
        }

        bool LoadFromExisting(const CAsset* const pCopy)
        {
            if(m_loaded) this->Destroy();
            if(pCopy == nullptr || !pCopy->IsLoaded()) return false;

            const unsigned char* raw =
                reinterpret_cast<const unsigned char*>(pCopy->GetData());

            const CTexture* const pCopyTexture =
                reinterpret_cast<const CTexture* const>(pCopy);

            bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA,
                pCopyTexture->m_width, pCopyTexture->m_height, raw);

            // new[] occured in GetData() so we are responsible for cleanup.
            delete[] raw;

            m_width = pCopyTexture->m_width;
            m_height= pCopyTexture->m_height;
            this->SetFilename(pCopyTexture->GetFilename());

            return (m_loaded = ret);
        }

        bool LoadFromRaw(const GLenum iformat, const GLenum format,
                         const uint16_t w, const uint16_t h,
                         const unsigned char* data)
        {
            if(m_loaded) this->Destroy();

            GL(glGenTextures(1, &m_texture));
            GL(glBindTexture(GL_TEXTURE_2D, m_texture));

            GL(glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, format,
                    GL_UNSIGNED_BYTE, data));

            GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

            GL(glBindTexture(GL_TEXTURE_2D, 0));

            m_width = w;
            m_height = h;

            std::stringstream ss;
            ss << "Raw texture data " << ((void*)data) << '.';
            this->SetFilename(ss.str());

            return (m_loaded = true);
        }

        const void* const GetData() const
        {
            this->Bind();

            // Size is width * height * bits per component
            // Since we force RBGA format, use 4.
            unsigned char* raw =
                new unsigned char[m_width * m_height * 4];

            GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw));
            return raw;
        }

        bool Bind() const
        {
            if(!m_loaded) return false;
            GL(glBindTexture(GL_TEXTURE_2D, m_texture));
            return true;
        }

        bool Unbind() const
        {
            if(!m_loaded) return false;
            GL(glBindTexture(GL_TEXTURE_2D, 0));
            return true;
        }

        uint16_t GetWidth()     const { return m_width; }
        uint16_t GetHeight()    const { return m_height; }

        friend class asset::CAssetManager;

    private:
        CTexture(const void* const owner = nullptr) : CAsset(owner),
            m_width(0), m_height(0) {}

        bool Destroy()
        {
            if(m_loaded)
            {
                glDeleteTextures(1, &m_texture);
                m_width = m_height = 0;
            }

            return !(m_loaded = false);
        }

        GLuint m_texture;
        GLuint m_width, m_height;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__TEXTURE_HPP

/** @} **/

/**
 * @class zen::gfxcore::CTexture
 * @description
 *  @a Zenderer uses `stb_image.c`, a single-file light-weight C library, 
 *  to load a variety of image formats. It has only been tested internally
 *  with 24-bit and 32-bit `.png` files. It will internally attempt force
 *  a conversion to a 32-bit RGBA format. If this doesn't succeed, it will
 *  obviously be apparent as soon as the texture is rendered.
 **/
