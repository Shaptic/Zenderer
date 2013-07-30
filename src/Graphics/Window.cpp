#include "Zenderer/Graphics/Window.hpp"

using namespace zen;
using namespace util;

using gfx::CWindow;

// Shortcut for Windows VSYNC toggling.
typedef bool (APIENTRY* PFNWGLSWAPINTERVALFARPROC)(int);
static PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

CWindow::CWindow(const uint16_t     width,
                 const uint16_t     height,
                 const string_t&    caption,
                 asset::CAssetManager& Mgr,
                 const bool         fullscreen) :
    CSubsystem("Window"), m_Log(CLog::GetEngineLog()),
    m_Assets(Mgr), m_Dimensions(width, height),
    m_caption(caption), m_clearbits(GL_COLOR_BUFFER_BIT),
    m_fullscreen(fullscreen) {}

CWindow::~CWindow()
{
    this->Destroy();
}

bool CWindow::Init()
{
    if(m_init)
    {
        m_Log   << m_Log.SetSystem("Window")
                << m_Log.SetMode(LogMode::ZEN_ERROR)
                << "Window is already initialized; destroy it first!"
                << CLog::endl;
        return false;
    }

    m_Log   << m_Log.SetSystem("Window")
            << m_Log.SetMode(LogMode::ZEN_INFO)
            << "Set up window (" << m_Dimensions.x << "x"
            << m_Dimensions.y << "): " << m_caption << CLog::endl;

    uint16_t major = (int)ZENDERER_GL_VERSION;
    uint16_t minor = floor(((ZENDERER_GL_VERSION - major) * 10) + 0.5);

    m_Log   << "Minimum OpenGL version: " << major << "." << minor
            << CLog::endl;

    // Set up window to be version major.minor and core profile only.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);

    // Create the window.
    mp_Window = glfwCreateWindow(m_Dimensions.x, m_Dimensions.y,
        m_caption.c_str(), m_fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr);

    if(mp_Window == nullptr)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL)
                << "Failed to create OpenGL window v" << major << '.' << minor
                << " (fullscreen = " << (m_fullscreen ? "true" : "false")
                << ")!" << CLog::endl;

        return false;
    }
    else
    {
        m_Log   << "Created OpenGL window v" << major << '.' << minor
                << " (fullscreen = " << (m_fullscreen ? "true" : "false")
                << ")." << CLog::endl;
    }

    glfwMakeContextCurrent(mp_Window);

    if(m_fullscreen)
        glfwSetInputMode(mp_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Registers event callbacks with GLFW.
    glfwSetCharCallback(mp_Window, evt::CEventHandler::CharacterCallback);
    glfwSetKeyCallback(mp_Window, evt::CEventHandler::KeyboardCallback);
    glfwSetMouseButtonCallback(mp_Window, evt::CEventHandler::MouseCallback);
    glfwSetCursorPosCallback(mp_Window, evt::CEventHandler::MouseMotionCallback);

    m_Log << "Initializing GLEW: ";
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << CLog::endl;
        return false;
    }
    else m_Log << "SUCCESS." << CLog::endl;

    // Clears a weird error of glewInit() giving 1280 (invalid enumerant)
    /// @see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
    glGetError();

    GL(glViewport(0, 0, m_Dimensions.x, m_Dimensions.y));

    m_ProjMatrix = math::matrix4x4_t::Projection2D(m_Dimensions.x,
                                                   m_Dimensions.y,
                                                   16, 1);

    gfxcore::CRenderer::s_ProjMatrix = m_ProjMatrix;
    return (m_init = gfxcore::CRenderer::Init(m_Assets,
                                              m_Dimensions.x,
                                              m_Dimensions.y));
}

bool CWindow::Destroy()
{
    if(!m_init) return true;

    delete gfxcore::CRenderer::s_DefaultMaterial;
    m_Assets.Destroy();
    glfwDestroyWindow(mp_Window);
    mp_Window = nullptr;
    return !(m_init = false);
}

bool CWindow::Clear()
{
    GL(glClear(m_clearbits));
    GL(glClearColor(0.f, 0.f, 0.f, 1.f));
    return true;
}

bool CWindow::Clear(const color4f_t& Color)
{
    GL(glClear(m_clearbits));
    GL(glClearColor(Color.r, Color.g, Color.b, Color.a));
    return true;
}

void CWindow::Update() const
{
    if(this->IsInit()) glfwSwapBuffers(mp_Window);
}

bool CWindow::ToggleFullscreen(int* const loaded)
{
    if(m_fullscreen) return this->DisableFullscreen(loaded);
    else             return this->EnableFullscreen(loaded);
}

bool CWindow::EnableFullscreen(int* const loaded)
{
    if(m_fullscreen) return true;

    this->Destroy();
    this->Init();
    m_fullscreen = true;
    uint32_t done = this->ReloadAssets();
    if(loaded != nullptr) *loaded = done;
    return done == m_Assets.GetAssetCount();
}

bool CWindow::DisableFullscreen(int* const loaded)
{
    if(!m_fullscreen) return true;

    this->Destroy();
    this->Init();
    m_fullscreen = false;
    uint32_t done = this->ReloadAssets();
    if(loaded != nullptr) *loaded = done;
    return done == m_Assets.GetAssetCount();
}

bool CWindow::IsOpen() const
{
    return !glfwWindowShouldClose(mp_Window);
}

bool CWindow::IsFullscreen() const
{
    return m_fullscreen;
}

void CWindow::Close() const
{
    glfwSetWindowShouldClose(mp_Window, true);
}

math::vector_t CWindow::GetMousePosition() const
{
    double x, y;
    glfwGetCursorPos(mp_Window, &x, &y);
    return math::vector_t(x, y);
}

bool CWindow::GetMouseState(const evt::MouseButton& Btn) const
{
    return glfwGetMouseButton(mp_Window, static_cast<int>(Btn)) == GLFW_PRESS;
}

/// @todo   Implement asset reloading from file.
uint32_t CWindow::ReloadAssets()
{
    this->Destroy();
    m_fullscreen = !m_fullscreen;
    this->Init();

    // Attempt to reload assets from their filenames.
    uint32_t done = 0;
    for(auto i = m_Assets.cbegin(); i != m_Assets.cend(); ++i)
    {
        if((*i)->Reload()) ++done;
    }

    return done;
}

bool CWindow::ToggleVSYNC()
{
    static bool on = true;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(on = !on);

    return on;
}
