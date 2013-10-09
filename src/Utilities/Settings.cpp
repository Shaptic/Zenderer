#include "Zenderer/Utilities/Settings.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

using util::zSettings;
using util::zOption;

/****************************************************************
 *                     zOption Definitions                      *
 ***************************************************************/

zOption::zOption(){}

zOption::zOption(const string_t& value) :
    m_value(value) {}

zOption::zOption(const zOption& Opt) :
    m_value(Opt.m_value) {}

zOption& zOption::operator=(const zOption& Opt)
{
    m_value = Opt.m_value;
    return (*this);
}

zOption& zOption::operator=(const string_t& name)
{
    m_value = name;
    return (*this);
}

zOption& zOption::operator=(const char* name)
{
    m_value = string_t(name);
    return (*this);
}

zOption& zOption::operator=(const bool name)
{
    m_value = name ? "1" : "0";
    return (*this);
}

bool zOption::operator==(const zOption& value) const
{
    return (value.m_value == m_value);
}

bool zOption::operator==(const string_t& value) const
{
    return (value == m_value);
}

bool zOption::operator==(const bool value) const
{
    return value ? m_value != "0" : m_value == "0";
}

/// Outputting an option value
std::ostream& util::operator<<(std::ostream& o, const zOption& Opt)
{
    return (o << Opt.m_value);
}

/***************************************************************
 *                      zSettings Definitions                  *
 ***************************************************************/

zSettings::zSettings(const string_t& filename) :
    zSubsystem("Settings"), m_Log(zLog::GetEngineLog()),
    m_filename(filename)
{}

zSettings::~zSettings() {}

bool zSettings::Init()
{
    if(m_filename.empty()) return (m_init = true);

    util::zParser Parser;
    if(!Parser.LoadFromFile(m_filename))
        return (m_init = false);

    // Copy the values from the key=value parser
    // into the internal options dictionary.
    const auto& dict = Parser.GetPairs();
    for(const auto& i : dict)
    {
        m_Options[
#ifdef ZEN_DEBUG_BUILD
            i.first
#else
            util::string_hash(i.first)
#endif  // ZEN_DEBUG_BUILD
        ] = i.second;
    }

    return (m_init = true);
}

bool zSettings::Destroy()
{
    m_Options.clear();
    return !(m_init = false);
}

zOption& zSettings::operator[](const string_t& opt)
{
    // Search via hash with release builds.
#ifndef ZEN_DEBUG_BUILD
    uint32_t hash = util::string_hash(opt);
#else
    const string_t& hash = opt;

    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("Settings") << "Accessing option '"
            << opt << "'." << zLog::endl;
#endif // ZEN_DEBUG_BUILD

    auto iter = m_Options.find(hash);
    if(iter == m_Options.end())
    {
        // Create a new blank option and return it.
#ifndef __GNUC__
        return m_Options.emplace(hash, zOption);
#else
        m_Options[hash] = zOption();
        return m_Options[hash];
#endif
    }

    return iter->second;
}
