#include "Zenderer/CoreGraphics/ShaderSet.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfxcore::CShaderSet;

CShaderSet::CShaderSet(asset::CAssetManager& Assets) :
    m_AssetManager(Assets), m_Log(CLog::GetEngineLog()),
    mp_FShader(nullptr), mp_VShader(nullptr), m_program(0),
    m_error_str("")
{}

CShaderSet::~CShaderSet()
{
    this->Destroy();
}

bool CShaderSet::LoadFromFile(const string_t& vs, const string_t& fs)
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

bool CShaderSet::LoadVertexShaderFromFile(const string_t& filename)
{
    //this->Destroy();

    mp_VShader = m_AssetManager.Create<CShader>(filename);
    if(mp_VShader == nullptr)
    {
        this->ShowLoadError(filename, "vertex");
        return false;
    }

    return true;
}

bool CShaderSet::LoadFragmentShaderFromFile(const string_t& filename)
{
    //this->Destroy();

    mp_FShader = m_AssetManager.Create<CShader>(filename);
    if(mp_FShader == nullptr)
    {
        this->ShowLoadError(filename, "fragment");
        return false;
    }

    return true;
}

bool CShaderSet::CreateShaderObject()
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
    m_program = GL(glCreateProgram());
    GL(glAttachShader(m_program, mp_VShader->GetShaderObject()));
    GL(glAttachShader(m_program, mp_FShader->GetShaderObject()));

    // Link the compiled shader objects to the program.
    GLint err = GL_NO_ERROR;
    GL(glLinkProgram(m_program));
    GL(glGetProgramiv(m_program, GL_LINK_STATUS, &err));

    int length  = 0;

    // Get log length to make an appropriate buffer.
    GL(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length));

    // Delete old log.
    m_error_str.clear();
    m_link_log.clear();

    // Get log.
    if(length > 0)
    {
        char* buffer = new char[length];
        GL(glGetProgramInfoLog(m_program, length, &length, buffer));

        m_link_log = buffer;
        delete[] buffer;

        m_Log << m_Log.SetMode(LogMode::ZEN_DEBUG) << m_Log.SetSystem("Shader")
              << "Shader compilation log: " << m_link_log << CLog::endl;
    }

    // Link failed?
    if(err == GL_FALSE)
    {
        GL(glDeleteProgram(m_program));
        m_error_str = m_link_log;

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

bool CShaderSet::Bind()
{
    if(m_program == 0)
    {
        this->ShowProgramError();
        return false;
    }

    GL(glUseProgram(m_program));
    return true;
}

bool CShaderSet::Unbind()
{
    if(m_program == 0)
    {
        this->ShowProgramError();
        return false;
    }

    GL(glUseProgram(0));
    return true;
}

uint16_t CShaderSet::GetShaderObject()
{
    return m_program;
}

short CShaderSet::GetUniformLocation(const string_t& name)
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

short CShaderSet::GetAttributeLocation(const string_t& name)
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

const string_t& CShaderSet::GetError() const
{
    return m_error_str;
}

bool CShaderSet::Destroy()
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("ShaderSet")
            << "Destroying shader set." << CLog::endl;

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
    return true;
}
