#include "Zenderer/CoreGraphics/ShaderSet.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using gfxcore::zShaderSet;

std::map<zShaderSet*, GLuint> zShaderSet::s_shaderPrograms;
uint16_t zShaderSet::s_ID = 1;

zShaderSet::zShaderSet(asset::zAssetManager& Assets) :
    zGLSubsystem("ShaderSet"),
    m_AssetManager(Assets), m_Log(zLog::GetEngineLog()),
    mp_FShader(nullptr), mp_VShader(nullptr), m_program(0),
    m_error_str(""), m_refcount(0), m_ID(0)
{}

zShaderSet::~zShaderSet()
{
    this->Destroy();
    ZEN_ASSERTM(m_program == 0 && !m_init, "didn't free GL program!");
}

zShaderSet::zShaderSet(const zShaderSet& Copy) :
    m_Log(Copy.m_Log),
    m_AssetManager(Copy.m_AssetManager),
    mp_FShader(Copy.mp_FShader),
    mp_VShader(Copy.mp_VShader),
    m_error_str(Copy.GetError()),
    m_link_log(Copy.GetLinkerLog()),
    m_program(Copy.m_program)
{
}

zShaderSet& zShaderSet::operator=(const zShaderSet& Copy)
{
    mp_FShader  = Copy.mp_FShader;
    mp_VShader  = Copy.mp_VShader;
    m_error_str = Copy.GetError();
    m_link_log  = Copy.GetLinkerLog();
    m_program   = Copy.m_program;

    return (*this);
}

bool zShaderSet::LoadFromFile(const string_t& vs, const string_t& fs)
{
    // Kill any existing shader programs.
    this->Destroy();

    mp_FShader = m_AssetManager.Create<zShader>(vs);
    mp_VShader = m_AssetManager.Create<zShader>(fs);

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

bool zShaderSet::LoadVertexShaderFromFile(const string_t& filename)
{
    this->DestroyVS();

    mp_VShader = m_AssetManager.Create<zShader>(filename);
    if(mp_VShader == nullptr)
    {
        this->ShowLoadError(filename, "vertex");
        return false;
    }

    return true;
}

bool zShaderSet::LoadFragmentShaderFromFile(const string_t& filename)
{
    this->DestroyFS();

    mp_FShader = m_AssetManager.Create<zShader>(filename);
    if(mp_FShader == nullptr)
    {
        this->ShowLoadError(filename, "fragment");
        return false;
    }

    return true;
}

bool zShaderSet::LoadVertexShaderFromStr(const string_t& str)
{
    this->DestroyVS();

    mp_VShader = m_AssetManager.Create<zShader>();
    mp_VShader->SetType(GL_VERTEX_SHADER);
    return mp_VShader->LoadFromRaw(str);
}

bool zShaderSet::LoadFragmentShaderFromStr(const string_t& str)
{
    this->DestroyFS();

    mp_FShader = m_AssetManager.Create<zShader>();
    mp_FShader->SetType(GL_FRAGMENT_SHADER);
    return mp_FShader->LoadFromRaw(str);
}

bool zShaderSet::CreateShaderObject()
{
    if(mp_FShader == nullptr || mp_VShader == nullptr)
    {
        m_error_str = "No shader objects loaded.";

        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("ShaderSet")
                << m_error_str << zLog::endl;

        return (m_init = false);
    }

    // Test for existing shader program.
    for(auto& i : s_shaderPrograms)
    {
        zShaderSet& SS = *(i.first);
        if(SS.mp_VShader == mp_VShader &&
           SS.mp_FShader == mp_FShader &&
           SS.m_program  != 0)
        {
            m_Log << m_Log.SetMode(LogMode::ZEN_DEBUG)
                  << m_Log.SetSystem("ShaderSet")
                  << "Found existing shader program for {'"
                  << mp_VShader->GetFilename() << "', '"
                  << mp_FShader->GetFilename() << "'}: " << SS.m_program
                  << '.' << zLog::endl;

            // We have a match, so copy the shader program handle
            // instead of creating a new one. It may have errors, so we
            // give feedback to the user accordingly.
            m_program   = SS.m_program;
            m_error_str = SS.m_error_str;
            m_link_log  = SS.m_link_log;
            m_ID        = SS.m_ID;
            m_refcount  = ++SS.m_refcount;
            return (m_init = m_error_str.empty());
        }
    }

    // Create shader program and attach shaders.
    m_program = GL(glCreateProgram());
    GL(glAttachShader(m_program, mp_VShader->GetShaderObject()));
    GL(glAttachShader(m_program, mp_FShader->GetShaderObject()));

    // Link the compiled shader objects to the program.
    GLint err = GL_NO_ERROR;
    GL(glLinkProgram(m_program));
    GL(glGetProgramiv(m_program, GL_LINK_STATUS, &err));

    int length = 0;

    // Get log length to make an appropriate buffer.
    GL(glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length));

    // Delete old log.
    m_error_str.clear();
    m_link_log.clear();

    // Get log.
    if(length > 1)
    {
        char* buffer = new char[length];
        GL(glGetProgramInfoLog(m_program, length, &length, buffer));

        m_link_log = buffer;
        delete[] buffer;

        m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << m_Log.SetSystem("ShaderSet") << "Shader linker log: "
                << m_link_log << zLog::endl;
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
                << m_error_str << "." << zLog::endl;

        this->Destroy();
        return (m_init = false);
    }

    // Add ourselves to the internal program storage.
    ++m_refcount;
    m_ID = s_ID++;

    ZEN_ASSERTM(s_ID < (1 << 6),
        "too many shader programs; material ID will not be unique");

    s_shaderPrograms[this] = m_program;
    return (m_init = true);
}

bool zShaderSet::Bind() const
{
    if(m_program == 0)
    {
        this->ShowProgramError();
        return false;
    }

    GL(glUseProgram(m_program));
    return true;
}

bool zShaderSet::Unbind() const
{
    if(m_program == 0)
    {
        this->ShowProgramError();
        return false;
    }

    GL(glUseProgram(0));
    return true;
}

uint16_t zShaderSet::GetShaderObject() const
{
    return m_program;
}

GLuint zShaderSet::GetObjectHandle() const
{
    return this->GetShaderObject();
}

GLint zShaderSet::GetUniformLocation(const string_t& name) const
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

GLint zShaderSet::GetAttributeLocation(const string_t& name) const
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

const string_t& zShaderSet::GetError() const
{
    return m_error_str;
}

const string_t& zShaderSet::GetLinkerLog() const
{
    return m_link_log;
}

bool zShaderSet::Destroy()
{
    if(!m_init) return false;

    this->DestroyFS();
    this->DestroyVS();

    if(m_program > 0 && --m_refcount == 0)
    {
        GL(glDeleteProgram(m_program));
    }

    m_error_str = "";
    m_ID = m_program = 0;
    return !(m_init = false);
}

bool zShaderSet::DestroyFS()
{
    if(mp_FShader != nullptr)
    {
        m_AssetManager.Delete(mp_FShader);
        mp_FShader = nullptr;
        return true;
    }

    return false;
}

bool zShaderSet::DestroyVS()
{
    if(mp_VShader != nullptr)
    {
        m_AssetManager.Delete(mp_VShader);
        mp_VShader = nullptr;
        return true;
    }

    return false;
}
