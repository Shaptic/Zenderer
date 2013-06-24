#include "Zenderer/Utilities/Log.hpp"

using namespace zen;
using util::CLog;

// Dat syntax...
CLog&               // Returns CLog
(
    CLog::*         // Pointer to member function of CLog
    CLog::endl      // Named CLog::endl (thus initializing this static member)
)()                 // No parameters for method
 = &CLog::Newline;  // Assigned to the address of CLog::Newline

CLog::CLog(const string_t& filename, const bool show_stdout) :
    m_mode(LogMode::ZEN_INFO),
    m_filename(filename),
    m_system("Log"),
    m_stdout(show_stdout),
    m_init(false)
{}

CLog::~CLog()
{
    this->Destroy();
}

bool CLog::Init()
{
    // Don't initialize again w/o a call to Destroy()
    if(this->IsInit()) return false;

    m_log.open(m_filename);
    if(!m_log) return false;

    time_t now;
    time(&now);

    m_log << "[INFO ] Log -- Log initialized on " << ctime(&now);
    this->Newline();

    return (m_init = true);
}

bool CLog::Destroy()
{
    // Can't destroy if we haven't initialized.
    if(!this->IsInit()) return false;

    this->Newline();

    time_t now;
    time(&now);

    m_mode = LogMode::ZEN_INFO;
    m_system = "Log";
    (*this) << "Log closed on " << ctime(&now) << CLog::endl;

    m_log.close();

    return !(m_init = false);
}

CLog& CLog::Newline()
{
    if(m_str.str().empty() || !this->IsInit()) return (*this);

#ifndef _DEBUG
    if(m_mode == util::LogMode::ZEN_DEBUG) return (*this);
#endif // _DEBUG

    std::stringstream tmp;

    tmp << "[";

    switch(m_mode)
    {
    case LogMode::ZEN_DEBUG:   tmp << "DEBUG"; break;
    case LogMode::ZEN_INFO:    tmp << "INFO "; break;
    case LogMode::ZEN_ERROR:   tmp << "ERROR"; break;
    case LogMode::ZEN_FATAL:   tmp << "FATAL"; break;
    default:                   tmp << "LOGGR"; break;
    }

    tmp << "] " << m_system << " -- " << m_str.str() << std::endl;

    m_log << tmp.str();
    if(m_stdout) std::cout << tmp.str();

    if(m_mode == LogMode::ZEN_FATAL)
    {
        error_fallback(tmp.str().c_str(), "Fatal Error");

        // If these aren't cleared, exit() calls ~CLog() which calls Destroy()
        // which calls Newline() so the message would be output twice.
        m_log.flush();
        m_str.str(std::string());
        exit(1);
    }

    m_log.flush();
    m_str.str(std::string());

    return (*this);
}

std::ostream& util::operator<<(std::ostream& o, const CLog& Log)
{
    return o;
}

CLog& CLog::GetEngineLog()
{
    static CLog Log("Zenderer.log");
    return Log;
}
