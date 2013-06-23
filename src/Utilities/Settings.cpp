#include "Zenderer/Utilities/Settings.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

using util::CSettings;
using util::COption;

/****************************************************************
 *                     COption Definitions                      *
 ***************************************************************/

COption::COption(){}

COption::COption(const string_t& value) :
    m_value(value) {}

COption::COption(const COption& Opt) :
    m_value(Opt.m_value) {}

COption& COption::operator=(const COption& Opt)
{
    m_value = Opt.m_value;
    return (*this);
}
            
COption& COption::operator=(const string_t& name)
{
    m_value = name;
    return (*this);
}

COption& COption::operator=(const char* name)
{
    m_value = string_t(name);
    return (*this);
}

COption& COption::operator=(const bool name)
{
    m_value = name ? "1" : "0";
    return (*this);
}

bool COption::operator==(const COption& value) const
{
    return (value.m_value == m_value);
}

bool COption::operator==(const string_t& value) const
{
    return (value == m_value);
}

bool COption::operator==(const bool value) const
{
    return value ? m_value != "0" : m_value == "0";
}

/// Outputting an option value
std::ostream& util::operator<<(std::ostream& o, const COption& Opt)
{
    return (o << Opt.m_value);
}

/****************************************************************
 *                      CSettings Definitions                   *
 ***************************************************************/

CSettings::CSettings(const string_t filename) :
    CSubsystem("Settings"), m_Log(CLog::GetEngineLog()),
    m_filename(filename)
{}

CSettings::~CSettings() {}

bool CSettings::Init()
{
    if(m_filename.empty()) return (m_init = true);
    
    util::CINIParser Parser;
    if(!Parser.LoadFromFile(m_filename))
        return (m_init = false);
    
    // Copy the values from the key=value parser
    // into the internal options dictionary.
    const auto& dict = Parser.GetPairs();
    for(const auto& i : dict)
    {
        m_Options[
#ifdef _DEBUG
            i.first
#else
            util::string_hash(i.first)
#endif  // _DEBUG
        ] = i.second;
    }
    
    return (m_init = true);
}

bool CSettings::Destroy()
{
    m_Options.clear();
    return !(m_init = false);
}

COption& CSettings::operator[](const string_t& opt)
{
    // Search via hash with release builds.
#ifndef _DEBUG
    uint32_t hash = util::string_hash(opt);
#else
    const string_t& hash = opt;
    
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("Settings") << "Accessing option '"
            << opt << "'." << CLog::endl;
#endif // _DEBUG

    auto iter = m_Options.find(hash);
    
    if(iter == m_Options.end())
    {
        // Create a new blank option and return it
        m_Options[hash] = COption();
        return m_Options[hash];
    }
    
    return iter->second;
}
