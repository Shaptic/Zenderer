#include "Zenderer/CoreGraphics/Texture.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using gfxcore::zTexture;

zTexture zTexture::s_DefaultTexture;
uint16_t zTexture::s_ID = 1;

zTexture::zTexture(const void* const owner) :
    zAsset(owner), m_width(0), m_height(0), m_TextureID(0)
{
    if(owner == nullptr) this->SetOwner(glfwGetCurrentContext());
}

zTexture::~zTexture()
{
    this->Destroy();
}

bool zTexture::LoadFromFile(const string_t& filename)
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
              << m_Log.SetSystem("Texture") << m_error_str << zLog::endl;

        stbi_image_free(raw);
        return (m_loaded = false);
    }

    bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA, w, h, raw);

    m_TextureID = s_ID++;

    ZEN_ASSERTM(s_ID < (1 << 10),
                "too many textures, material ID can't be unique");

    stbi_image_free(raw);
    this->SetFilename(filename);
    return ret && (m_loaded = true);
}

bool zTexture::LoadFromExisting(const zAsset* const pCopy)
{
    if(pCopy == nullptr || !pCopy->IsLoaded() || this == pCopy) return false;
    if(m_loaded) this->Destroy();

    const unsigned char* raw =
        static_cast<const unsigned char*>(pCopy->GetData());

    const zTexture* const pCopyTexture =
        static_cast<const zTexture* const>(pCopy);

    bool ret = this->LoadFromRaw(GL_RGBA8, GL_RGBA,
        pCopyTexture->m_width, pCopyTexture->m_height, raw);

    // new[] occured in GetData() so we are responsible for cleanup.
    delete[] raw;

    m_width = pCopyTexture->m_width;
    m_height= pCopyTexture->m_height;
    m_TextureID    = pCopyTexture->m_TextureID;
    this->SetFilename(pCopyTexture->GetFilename());

    return (m_loaded = ret);
}

bool zTexture::LoadFromExisting(const GLuint handle)
{
    GLint w, h;
    GL(glBindTexture(GL_TEXTURE_2D, handle));
    GL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w));
    GL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h));
    GL(glBindTexture(GL_TEXTURE_2D, 0));

    if(w <= 0 || h <= 0) return false;
    m_width = w; m_height = h;
    std::stringstream ss;
    ss << "Texture handle " << handle;
    this->SetFilename(ss.str());
    return m_loaded = ((m_texture = handle) != 0);
}

bool zTexture::CopyFromExisting(const GLuint handle)
{
    if(handle == 0) return false;

    GLint w, h;
    GL(glBindTexture(GL_TEXTURE_2D, handle));
    GL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w));
    GL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h));

    unsigned char* raw = new unsigned char[w * h * 4];

    GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw));
    GL(glBindTexture(GL_TEXTURE_2D, 0));

    return this->LoadFromRaw(GL_RGBA8, GL_RGBA, w, h, raw);
}

bool zTexture::LoadFromRaw(const GLenum iformat, const GLenum format,
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
    ss << "Raw texture data " << ((void*)data);
    this->SetFilename(ss.str());

    return (m_loaded = true);
}

const void* const zTexture::GetData() const
{
    this->Bind();

    // Size is width * height * bits per component
    // Since we force RBGA format, use 4.
    unsigned char* raw = new unsigned char[m_width * m_height * 4];

    GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw));
    return static_cast<const void* const>(raw);
}

bool zTexture::Bind() const
{
    if(!m_loaded) return false;
    GL(glBindTexture(GL_TEXTURE_2D, m_texture));
    return true;
}

bool zTexture::Unbind() const
{
    if(!m_loaded) return false;
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    return true;
}

zTexture& zTexture::GetDefaultTexture()
{
    if(s_DefaultTexture.IsLoaded()) return s_DefaultTexture;

    // Load the default texture (1x1 white pixel).
    static const unsigned char white[] = {'\xff', '\xff', '\xff', '\xff'};
    s_DefaultTexture.LoadFromRaw(GL_RGBA8, GL_RGBA, 1, 1, white);
    s_DefaultTexture.SetFilename("Zenderer white texture");
    return s_DefaultTexture;
}

bool zTexture::Destroy()
{
    if(m_loaded)
    {
        glDeleteTextures(1, &m_texture);
        m_width = m_height = m_texture = 0;
    }

    return !(m_loaded = false);
}
