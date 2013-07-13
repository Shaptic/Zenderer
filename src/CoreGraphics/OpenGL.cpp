#include "Zenderer/CoreGraphics/OpenGL.hpp"

using namespace zen::gfxcore;

std::vector<CGLSubsystem*> CGLSubsystem::sp_allGLSystems;

CGLSubsystem::CGLSubsystem(const zen::string_t name) :
    m_init(false), m_name(name)
{
    sp_allGLSystems.push_back(this);
}

CGLSubsystem::~CGLSubsystem()
{
    for(auto i = sp_allGLSystems.begin(); i != sp_allGLSystems.end(); ++i)
    {
        if(*i == this)
        {
            sp_allGLSystems.erase(i);
            break;
        }
    }
}
