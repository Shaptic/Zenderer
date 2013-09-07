#include "Zenderer/Core/Subsystem.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

std::vector<zSubsystem*> zSubsystem::sp_allSystems;

zSubsystem::zSubsystem(const string_t name) :
    m_Log(zLog::GetEngineLog()),
    m_name(name), m_init(false)
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO) << m_Log.SetSystem(name)
            << "Created component." << zLog::endl;

    sp_allSystems.push_back(this);
}

zSubsystem::~zSubsystem()
{
    // Use log since it's destroyed last.
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem(this->GetName())
            << "Destroyed component." << zLog::endl;

    m_init = false;
}
