#include "Zenderer/CoreGraphics/Texture.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfxcore::CTexture;

CTexture::CTexture(const void* const owner) :
    CAsset(owner), m_width(0), m_height(0) {}

CTexture::~CTexture()
{
    this->Destroy();
}
    
bool CTexture::LoadFromFile(const string_t& filename)
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
              << CLog::endl;

        return (m_loaded = false);
    }

    bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA, w, h, raw);

    stbi_image_free(raw);
    this->SetFilename(filename);
    return (m_loaded = true);
}

bool CTexture::LoadFromExisting(const CAsset* const pCopy)
{
    if(m_loaded) this->Destroy();
    if(pCopy == nullptr || !pCopy->IsLoaded()) return false;

    const unsigned char* raw =
        static_cast<const unsigned char*>(pCopy->GetData());

    const CTexture* const pCopyTexture =
        static_cast<const CTexture* const>(pCopy);

    bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA,
        pCopyTexture->m_width, pCopyTexture->m_height, raw);

    // new[] occured in GetData() so we are responsible for cleanup.
    delete[] raw;

    m_width = pCopyTexture->m_width;
    m_height= pCopyTexture->m_height;
    this->SetFilename(pCopyTexture->GetFilename());

    return (m_loaded = ret);
}

/// @todo Calculate texture w/h.
bool zen::gfxcore::CTexture::LoadFromExisting(const GLuint handle)
{
    m_width = m_height = 0;
    this->SetFilename("Texture handle");
    return m_loaded = ((m_texture = handle) != 0);
}

bool CTexture::LoadFromRaw(const GLenum iformat, const GLenum format,
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

const void* const CTexture::GetData() const
{
    this->Bind();

    // Size is width * height * bits per component
    // Since we force RBGA format, use 4.
    unsigned char* raw =
        new unsigned char[m_width * m_height * 4];

    GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw));
    return static_cast<const void* const>(raw);
}

bool CTexture::Bind() const
{
    if(!m_loaded) return false;
    GL(glBindTexture(GL_TEXTURE_2D, m_texture));
    return true;
}

bool CTexture::Unbind() const
{
    if(!m_loaded) return false;
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    return true;
}

bool CTexture::Destroy()
{
    if(m_loaded)
    {
        glDeleteTextures(1, &m_texture);
        m_width = m_height = 0;
    }

    return !(m_loaded = false);
}
