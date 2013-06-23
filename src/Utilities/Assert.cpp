#include "Zenderer/Utilities/Assert.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

void zen::util::runtime_assert(     const bool expression,
    const string_t expression_str,  const size_t line_no,
    const string_t file_name,       const char* msg)
{
    if(expression) return;
    
    util::CLog& Log = util::CLog::GetEngineLog();
    
    // Flush old output (if any).
    Log << CLog::endl;
    
    // Set up proper logging types.
    Log.SetSystem("Assert");
    
    // Make sure we have valid input.
    if(expression_str.empty() || file_name.empty())
    {
        Log.SetMode(LogMode::ZEN_ERROR);
        Log << "Invalid assertion arguments." << CLog::endl;
        return;
    }
    
    // Output as such:
    // [FATAL] Assert -- expression_str failed at line line_no of file_name: _msg_
    Log.SetMode(LogMode::ZEN_FATAL);
    Log << expression_str << " failed at line " << line_no
        << " of " << file_name;
    if(msg) Log << ": " << msg;
    Log << CLog::endl;
    
    // util::CLog::Newline() should take care of this (since FATAL),
    // but just in case.
    exit(1);
}