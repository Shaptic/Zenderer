#include "Zenderer/CoreGraphics/Shader.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfxcore::CShader;

CShader::CShader(const void* const ptr) :
    CAsset(ptr), m_object(0), m_type(GL_VERTEX_SHADER)
{}

CShader::~CShader()
{
    this->Destroy();
}

bool CShader::LoadFromFile(const string_t& filename)
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

bool CShader::LoadFromExisting(const CAsset* const pCopyShader)
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

bool CShader::LoadFromRaw(const string_t& string)
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

    // Retrieve log (if any).
    GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

    // Delete old logs
    m_error_str = m_shader_log = "";

    if(length > 0)
    {
        char* buffer = new char[length];

        GL(glGetShaderInfoLog(shader, length, &length, buffer));
        GL(glDeleteShader(shader));

        m_error_str = buffer;
        m_shader_log = buffer;

        delete[] buffer;
        buffer = nullptr;

#ifdef _DEBUG
        m_Log << m_Log.SetMode(LogMode::ZEN_DEBUG) << m_Log.SetSystem("Shader")
              << "Shader compilation log: " << m_shader_log << CLog::endl;
#endif // _DEBUG
    }

    // We have an error
    if(error_code == GL_FALSE)
    {
        ZEN_ASSERT(length > 0);

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

bool CShader::Destroy()
{
    if(m_object > 0)
    {
        GL(glDeleteShader(m_object));

        m_filename = "";
        m_object = m_filename_hash = m_type = 0;
    }

    return !(m_loaded = false);
}

const void* const CShader::GetData() const
{
    return reinterpret_cast<const void* const>(m_object);
}

GLuint CShader::GetShaderObject()
{
    return m_object;
}

const string_t& CShader::GetShaderLog()
{
    return m_shader_log;
}
