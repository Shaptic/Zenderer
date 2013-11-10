#include "Zenderer/Audio/Audio2D.hpp"

using namespace zen;
using sfx::zAudio2D;

using util::zLog;
using util::LogMode;

zAudio2D::zAudio2D(const void* const owner,
                   const void* const settings) :
    zAsset(owner), m_error_code(AL_NO_ERROR), m_loop(false)
{
    m_AL.buffers = new ALuint[1];
    m_AL.buffer_count = 1;
    m_AL.volume = 0.75;
    m_AL.source = 0;
}

zAudio2D::~zAudio2D()
{
    delete[] m_AL.buffers;
}

int zAudio2D::GetAudioState() const
{
    if(m_AL.source == 0) return -1;

    int state;
    AL(alGetSourcei(m_AL.source, AL_SOURCE_STATE, &state));
    return state;
}

const void* const zAudio2D::GetData() const
{
    return &m_AL;
}

void zAudio2D::UnloadSource()
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("Audio")
            << "Unloading source (" << m_AL.source << ")." << zLog::endl;

    zAudioManager::FreeSource(m_AL.source);
}
