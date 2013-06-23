#include "Zenderer/Graphics/Window.hpp"

using namespace zen;
using namespace util;

using gfx::CWindow;

CWindow::CWindow(const uint16_t     width,
                 const uint16_t     height,
                 const string_t&    caption) :
    CSubsystem("Window"), m_Log(CLog::GetEngineLog()),
    mp_Assets(nullptr), m_Dimensions(width, height),
    m_caption(caption), m_clearbits(GL_COLOR_BUFFER_BIT),
    m_fullscreen(
#ifdef _DEBUG
        false)
#else
        true)
#endif // _DEBUG
{}

CWindow::~CWindow()
{
    this->Destroy();
}

bool CWindow::Init()
{
    m_Log   << m_Log.SetSystem("Window")
            << m_Log.SetMode(LogMode::ZEN_INFO)
            << "Set up window (" << m_Dimensions.x << "x"
            << m_Dimensions.y << "): " << m_caption << CLog::endl;

    uint16_t major = (int)ZENDERER_GL_VERSION;
    uint16_t minor = floor(((ZENDERER_GL_VERSION - major) * 10) + 0.5);

    m_Log   << "Minimum OpenGL version: " << major << "." << minor
            << CLog::endl;

    // Set up window to be version major.minor and core profile only.
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, major);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, minor);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window.
    if(glfwOpenWindow(800, 600, 8, 8, 8, 8,
        24, 8, (m_fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)) == GL_FALSE)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL)
                << "Failed to create OpenGL window v" << major
                << '.' << minor << " (fullscreen = "
                << (m_fullscreen ? "true" : "false") << ")!"
                << CLog::endl;

        return false;
    }

    glfwSetWindowTitle(m_caption.c_str());

    if(m_fullscreen) glfwDisable(GLFW_MOUSE_CURSOR);

    m_ProjMatrix = math::matrix4x4_t::Projection2D(m_Dimensions.x,
        m_Dimensions.y, 256, -256);

    if(glewInit() != GLEW_OK) return false;

    // Clears a weird error of glewInit() giving 1280 (invalid enumerant)
    /// @see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
    glGetError();

    GL(glViewport(0, 0, m_Dimensions.x, m_Dimensions.y));

    return (m_init = true);
}

bool CWindow::Destroy()
{
    bool passed = true;
    if(mp_Assets != nullptr)
    {
        for(size_t i = 0; i < mp_Assets->GetAssetCount(); ++i)
        {
            if(!mp_Assets->Delete(i)) passed = false;
        }
    }

    glfwCloseWindow();

    return passed;
}

void CWindow::Clear()
{
    GL(glClear(m_clearbits));
    GL(glClearColor(0.f, 0.f, 0.f, 1.f));
}

void CWindow::Clear(const color4f_t& Color)
{
    GL(glClear(m_clearbits));
    GL(glClearColor(Color.r, Color.g, Color.b, Color.a));
}

void CWindow::AttachAssetManager(asset::CAssetManager& Mgr)
{
    mp_Assets = &Mgr;
}

void CWindow::Update() const
{
    if(this->IsInit()) glfwSwapBuffers();
}
