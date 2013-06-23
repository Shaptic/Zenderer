#include "Zenderer/CoreGraphics/OpenGL.hpp"

using namespace zen::gfxcore;

std::vector<CGLSubsystem*> CGLSubsystem::sp_allGLSystems;

CGLSubsystem::CGLSubsystem() : m_init(false)
{
    sp_allGLSystems.push_back(this);
}
