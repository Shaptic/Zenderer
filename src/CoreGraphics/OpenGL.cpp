#include "Zenderer/CoreGraphics/OpenGL.hpp"

using namespace zen::gfxcore;

std::vector<CGLSubsystem*> CGLSubsystem::sp_allGLSystems;

CGLSubsystem::CGLSubsystem(zen::string_t name) :
    m_init(false), m_name(name)
{
    sp_allGLSystems.push_back(this);
}
