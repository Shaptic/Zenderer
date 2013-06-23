#include "Zenderer/Core/Subsystem.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

std::vector<CSubsystem*> CSubsystem::sp_allSystems;

CSubsystem::CSubsystem(const string_t name) :
    m_Log(CLog::GetEngineLog()),
    m_name(name), m_init(false)
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO) << m_Log.SetSystem(name)
            << "Created component." << CLog::endl;

    sp_allSystems.push_back(this);
}

CSubsystem::~CSubsystem()
{
    // Use log since it's destroyed last.
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem(this->GetName())
            << "Destroyed component." << CLog::endl;

    m_init = false;
}
