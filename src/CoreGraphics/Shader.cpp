#include "Zenderer/CoreGraphics/Shader.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using gfxcore::zShader;

zShader::zShader(const void* const ptr) :
    zAsset(ptr), m_object(0), m_type(GL_VERTEX_SHADER)
{}

zShader::~zShader()
{
    this->Destroy();
}

bool zShader::LoadFromFile(const string_t& filename)
{
    if(m_loaded) this->Destroy();

    std::ifstream file(filename);
    std::stringstream source;
    std::string line;

    if(!file)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Shader") << "Failed to open '"
                << filename << "'." << zLog::endl;

        return false;
    }

    // Interpret type from filename (VS by default).
    if(filename.find(".fs") != string_t::npos)
    {
        m_type = GL_FRAGMENT_SHADER;
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

bool zShader::LoadFromExisting(const zAsset* const pCopyShader)
{
    // The given parameter must must must be a zShader* instance
    // in actuality. There is no way to test for this.

    const zShader* const pCopy =
        static_cast<const zShader* const>(pCopyShader);

    ZEN_ASSERT(pCopyShader != nullptr);
    ZEN_ASSERT(pCopy != nullptr);

    m_object = reinterpret_cast<decltype(m_object)>(pCopyShader->GetData());
    m_type = pCopy->m_type;

    return zAsset::LoadFromExisting(pCopyShader);
}

bool zShader::LoadFromRaw(const string_t& string)
{
    ZEN_ASSERT(!string.empty());

    const char* src = string.c_str();

    // Create shader object.
    GLuint shader = GL(glCreateShader(m_type));

    // Compile
    GLint error_code = GL_NO_ERROR;
    GLint length = string.length();
    GL(glShaderSource(shader, 1, &src, &length));

    ZEN_ASSERT(static_cast<size_t>(length) == string.length());

    GL(glCompileShader(shader));
    GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &error_code));

    // Retrieve log (if any).
    GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

    // Delete old logs
    m_error_str = m_shader_log = "";

    if(length > 1)
    {
        char* buffer = new char[length];

        GL(glGetShaderInfoLog(shader, length, &length, buffer));

        m_shader_log = buffer;

        delete[] buffer;
        buffer = nullptr;

        m_Log << m_Log.SetMode(LogMode::ZEN_DEBUG) << m_Log.SetSystem("Shader")
              << "Shader compilation log: " << m_shader_log << zLog::endl;
    }

    // We have an error
    if(error_code == GL_FALSE)
    {
        ZEN_ASSERT(length > 0);

        GL(glDeleteShader(shader));
        m_error_str = m_shader_log;

        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Shader") << "Failed to compile shader: "
                << m_error_str << zLog::endl;

        return (m_loaded = false);
    }

    m_object = shader;
    this->SetFilename("Raw shader string");
    m_error_str.clear();

    return (m_loaded = true);
}

bool zShader::Destroy()
{
    if(m_object > 0)
    {
        GL(glDeleteShader(m_object));

        m_filename = "";
        m_object = m_filename_hash = m_type = 0;
    }

    return !(m_loaded = false);
}

const void* const zShader::GetData() const
{
    return reinterpret_cast<const void* const>(m_object);
}

GLuint zShader::GetShaderObject() const
{
    return m_object;
}

const string_t& zShader::GetShaderLog() const
{
    return m_shader_log;
}

void zShader::SetType(const GLenum shader_type)
{
    m_type = shader_type;
}
