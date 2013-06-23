#include "Zenderer/Audio/Audio2D.hpp"

using namespace zen;
using sfx::CAudio2D;

using util::CLog;
using util::LogMode;

CAudio2D::CAudio2D() : 
    m_error_code(AL_NO_ERROR), m_loop(false)
{
    m_AL.buffers = new ALuint[1];
    m_AL.buffer_count = 1;
    m_AL.volume = 0.75;
    m_AL.source = 0;
}

CAudio2D::~CAudio2D()
{
    delete[] m_AL.buffers;
}

int CAudio2D::GetAudioState() const
{
    if(m_AL.source == 0) return -1;
    
    int state;
    AL(alGetSourcei(m_AL.source, AL_SOURCE_STATE, &state));
    return state;
}

const void* const CAudio2D::GetData() const
{
    return &m_AL;
}

void CAudio2D::UnloadSource()
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("Audio")
            << "Unloading source (" << m_AL.source << ")." << CLog::endl;

    CAudioManager::FreeSource(m_AL.source);
}
