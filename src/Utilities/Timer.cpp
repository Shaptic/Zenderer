#include "Zenderer/Utilities/Timer.hpp"

using std::chrono::duration_cast;

using zen::util::CTimer;
using zen::util::time_t;

CTimer::CTimer(const uint16_t frames) : m_fps(frames) {}

CTimer::~CTimer(){}

time_t CTimer::Start()
{
    m_start = clock_t::now();
    return duration_cast<precision_t>(m_start.time_since_epoch()).count();
}

time_t CTimer::Finish()
{
    m_end = clock_t::now();
    return duration_cast<precision_t>(m_end.time_since_epoch()).count();
}

time_t CTimer::Elapsed()
{
    return duration_cast<precision_t>(m_end - m_start).count();
}

/// Sleeps the current thread (defaults to milliseconds). 
template<typename T = precision_t>
void CTimer::Sleep<T>(const time_t ticks)
{
    std::this_thread::sleep_for(timepoint_t<T>::duration(ticks));
}

time_t CTimer::Delay()
{
    timepoint_t::duration pause = clock_t::now() - m_start;
    time_t ticks = duration_cast<precision_t>(pause).count();
    
    if(ticks < 1000.0 / m_fps)
        std::this_thread::sleep_for(pause);
        
    return ticks;
}

time_t CTimer::GetTime() const
{
    return duration_cast<precision_t>(
        clock_t::now().time_since_epoch()).count();
}

void CTimer::SetFrameRate(const uint16_t fps)
{
    m_fps = fps;
}