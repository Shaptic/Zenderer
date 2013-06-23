#include "Zenderer/Core/Subsystem.hpp"

using namespace zen;

USING_ZENDERER_LOG

std::vector<CSubsystem*> CSubsystem::sp_allSystems;

CSubsystem::CSubsystem(const string_t name) :
    m_name(name), m_init(false), mp_Log(nullptr)
{
    // Fake like log output (static initialization order).
    std::cout << "[INFO ] " << name << " -- Created component." << std::endl;

    sp_allSystems.push_back(this);
}

CSubsystem::~CSubsystem()
{
    // Use log since it's destroyed last.
    g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_INFO)
                << g_EngineLog.SetSystem(this->GetName())
                << "Destroyed component." << CLog::endl;

    m_init = false;
}
