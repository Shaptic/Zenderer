#include "Zenderer/Graphics/Window.hpp"

using namespace zen;
using namespace util;

using gfx::zWindow;

// Shortcut for Windows VSYNC toggling.
typedef bool (APIENTRY* PFNWGLSWAPINTERVALFARPROC)(int);
static PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

zWindow::zWindow(const uint16_t     width,
                 const uint16_t     height,
                 const string_t&    caption,
                 asset::zAssetManager& Mgr,
                 const bool         fullscreen) :
    zSubsystem("Window"), m_Log(zLog::GetEngineLog()),
    m_Assets(Mgr), m_Dimensions(width, height),
    m_caption(caption), m_clearbits(GL_COLOR_BUFFER_BIT),
    m_fullscreen(fullscreen) {}

zWindow::~zWindow()
{
    this->Destroy();
}

bool zWindow::Init()
{
    if(m_init)
    {
        m_Log   << m_Log.SetSystem("Window")
                << m_Log.SetMode(LogMode::ZEN_ERROR)
                << "Window is already initialized; destroy it first!"
                << zLog::endl;
        return false;
    }

    // Initialize the asset manager if it hasn't been already.
    m_Assets.Init();

    m_Log   << m_Log.SetSystem("Window")
            << m_Log.SetMode(LogMode::ZEN_INFO)
            << "Set up window (" << m_Dimensions.x << "x"
            << m_Dimensions.y << "): " << m_caption << zLog::endl;

    uint16_t major = (int)ZENDERER_GL_VERSION;
    uint16_t minor = floor(((ZENDERER_GL_VERSION - major) * 10) + 0.5);

    m_Log   << "Minimum OpenGL version: " << major << "." << minor
            << zLog::endl;

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
                << ")!" << zLog::endl;

        return false;
    }
    else
    {
        m_Log   << "Created OpenGL window v" << major << '.' << minor
                << " (fullscreen = " << (m_fullscreen ? "true" : "false")
                << ")." << zLog::endl;
    }

    glfwMakeContextCurrent(mp_Window);

    if(m_fullscreen)
        glfwSetInputMode(mp_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Registers event callbacks with GLFW.
    glfwSetCharCallback(mp_Window, evt::zEventHandler::CharacterCallback);
    glfwSetKeyCallback(mp_Window, evt::zEventHandler::KeyboardCallback);
    glfwSetMouseButtonCallback(mp_Window, evt::zEventHandler::MouseCallback);
    glfwSetCursorPosCallback(mp_Window, evt::zEventHandler::MouseMotionCallback);
    glfwSetWindowCloseCallback(mp_Window, evt::zEventHandler::WindowCloseCallback);

    m_Log << "Initializing GLEW: ";
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << zLog::endl;
        return false;
    }
    else m_Log << "SUCCESS." << zLog::endl;

    // Clears a weird error of glewInit() giving 1280 (invalid enumerant)
    /// @see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
    glGetError();

    GL(glViewport(0, 0, m_Dimensions.x, m_Dimensions.y));

    m_ProjMatrix = math::matrix4x4_t::Projection2D(m_Dimensions.x,
                                                   m_Dimensions.y,
                                                   16, 1);

    gfxcore::zRenderer::s_ProjMatrix = m_ProjMatrix;
    return (m_init = gfxcore::zRenderer::Init(m_Assets,
                                              m_Dimensions.x,
                                              m_Dimensions.y));
}

bool zWindow::Destroy()
{
    if(!m_init) return true;

    m_Assets.Destroy();

    // We're special.
    const_cast<zEffect&>(gfxcore::zRenderer::GetDefaultEffect()).Destroy();
    delete gfxcore::zRenderer::s_DefaultMaterial;

    for(auto& i : gfxcore::zGLSubsystem::sp_allGLSystems)
        i->Destroy();

    glfwDestroyWindow(mp_Window);
    mp_Window = nullptr;
    return !(m_init = false);
}

bool zWindow::Clear()
{
    GL(glClear(m_clearbits));
    GL(glClearColor(0.f, 0.f, 0.f, 1.f));
    return true;
}

bool zWindow::Clear(const color4f_t& Color)
{
    GL(glClear(m_clearbits));
    GL(glClearColor(Color.r, Color.g, Color.b, Color.a));
    return true;
}

void zWindow::Update() const
{
    if(this->IsInit()) glfwSwapBuffers(mp_Window);
}

bool zWindow::ToggleFullscreen(int* const loaded)
{
    if(m_fullscreen) return this->DisableFullscreen(loaded);
    else             return this->EnableFullscreen(loaded);
}

bool zWindow::EnableFullscreen(int* const loaded)
{
    if(m_fullscreen) return true;

    this->Destroy();
    this->Init();
    m_fullscreen = true;
    uint32_t done = this->ReloadAssets();
    if(loaded != nullptr) *loaded = done;
    return done == m_Assets.GetAssetCount();
}

bool zWindow::DisableFullscreen(int* const loaded)
{
    if(!m_fullscreen) return true;

    this->Destroy();
    this->Init();
    m_fullscreen = false;
    uint32_t done = this->ReloadAssets();
    if(loaded != nullptr) *loaded = done;
    return done == m_Assets.GetAssetCount();
}

bool zWindow::IsOpen() const
{
    return this->IsInit();
}

bool zWindow::IsFullscreen() const
{
    return m_fullscreen;
}

void zWindow::Close()
{
    this->Destroy();
}

math::vector_t zWindow::GetMousePosition() const
{
    double x, y;
    glfwGetCursorPos(mp_Window, &x, &y);
    return math::vector_t(x, y);
}

bool zWindow::GetMouseState(const evt::MouseButton& Btn) const
{
    return glfwGetMouseButton(mp_Window, static_cast<int>(Btn)) == GLFW_PRESS;
}

/// @todo   Implement asset reloading from file.
uint32_t zWindow::ReloadAssets()
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

bool zWindow::ToggleVSYNC()
{
    static bool on = true;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(on = !on);

    return on;
}
