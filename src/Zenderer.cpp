#include "Zenderer/Zenderer.hpp"

using zen::util::zLog;
using zen::util::LogMode;

using zen::sfx::zAudioManager;

bool zen::Init()
{
    // Seed random number generator.
    srand(time(nullptr));

    if(!zLog::GetEngineLog().Init())
        return false;

    zLog& Log = zLog::GetEngineLog();

    Log << zLog::endl;
    Log << Log.SetMode(LogMode::ZEN_INFO) << Log.SetSystem("Zenderer")
        << "Initializing Zenderer." << zLog::endl;

    Log << "Initializing GLFW: ";
    if(glfwInit() == GL_FALSE)
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << zLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << zLog::endl;
    }

    Log << "Initializing OpenAL: ";
    if(!zAudioManager::Init())
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << zLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << util::zLog::endl;
    }

    Log << "Initializing FreeType: ";
    if(!gui::zFontLibrary::InitFreetype().IsInit())
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << zLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << util::zLog::endl;
    }

    Log << "Initializing ";

#ifdef _WIN32
    Log << "WinSock: ";
#else
    Log << "socket API: ";
#endif // _WIN32
    if(!net::zSocket::InitializeLibrary())
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << zLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << util::zLog::endl;
    }

    for(auto& i : zSubsystem::sp_allSystems)
    {
        Log << "Initializing global subsystem (" << i->GetName() << "): ";
        if(!i->Init())
        {
            i->Destroy();
            Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED."
                << zLog::endl;
            return false;
        }
        else
        {
            Log << "SUCCESS." << zLog::endl;
        }
    }

    return true;
}

void zen::Quit()
{
    zLog& Log = zLog::GetEngineLog();

    gfxcore::zRenderer::GetDefaultEffect().Destroy();

    Log << Log.SetMode(LogMode::ZEN_INFO) << Log.SetSystem("Zenderer")
        << "Destroying OpenGL components." << zLog::endl;

    for(auto it  = zen::gfxcore::zGLSubsystem::sp_allGLSystems.rbegin();
             it != zen::gfxcore::zGLSubsystem::sp_allGLSystems.rend();
           ++it)
    {
        auto& sys = *it;
        Log << Log.SetMode(LogMode::ZEN_INFO)
            << Log.SetSystem(sys->GetName())
            << "Destroying component." << zLog::endl;

        if(!sys->Destroy())
        {
            Log.SetMode(LogMode::ZEN_ERROR);
            Log << "Failed to destroy component." << zLog::endl;
        }
    }

    Log << Log.SetMode(LogMode::ZEN_INFO) << Log.SetSystem("Zenderer")
        << "Destroying components." << zLog::endl;

    for(auto& i : zen::zSubsystem::sp_allSystems)
    {
        Log << Log.SetMode(LogMode::ZEN_INFO)
            << Log.SetSystem(i->GetName())
            << "Destroying component." << zLog::endl;

        if(!i->Destroy())
        {
            Log.SetMode(LogMode::ZEN_ERROR);
            Log << "Failed to destroy component." << zLog::endl;
        }
    }

    glfwTerminate();
    Log.Destroy();
}
