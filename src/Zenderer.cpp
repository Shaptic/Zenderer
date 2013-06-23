#include "Zenderer/Zenderer.hpp"

using zen::util::CLog;
using zen::util::LogMode;

using zen::sfx::CAudioManager;

bool zen::Init()
{
    if(!CLog::GetEngineLog().Init())
        return false;

    CLog& Log = CLog::GetEngineLog();
    
    Log << Log.SetMode(LogMode::ZEN_INFO) << Log.SetSystem("Init")
        << "Initializing Zenderer." << CLog::endl;

    Log << "Initializing GLFW: ";
    if(glfwInit() == GL_FALSE)
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << CLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << CLog::endl;
    }

    Log << "Initializing OpenAL: ";
    if(!CAudioManager::Init())
    {
        Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED." << CLog::endl;
        return false;
    }
    else
    {
        Log << "SUCCESS." << util::CLog::endl;
    }

    for(auto* i : CSubsystem::sp_allSystems) 
    {
        Log << "Initializing global subsystem (" << i->GetName() << "): ";
        if(!i->Init()) 
        {
            i->Destroy();
            Log << Log.SetMode(LogMode::ZEN_FATAL) << "FAILED."
                << CLog::endl;
            return false;
        }
        else
        {
            Log << "SUCCESS." << CLog::endl;
        }
    }

    return true;
}

void zen::Quit()
{
    CLog& Log = CLog::GetEngineLog();
    
    Log << Log.SetMode(LogMode::ZEN_INFO) << Log.SetSystem("Quit")
        << "Destroying components." << CLog::endl;

    for(auto* sys : zen::CSubsystem::sp_allSystems)
    {
        if(!sys->Destroy())
        {
            Log << Log.SetMode(LogMode::ZEN_ERROR) << Log.SetSystem("Quit")
                << "Failed to destroy component: " << sys->GetName() 
                << CLog::endl;
        }
    }

    Log.Destroy();
}

