

#ifndef ZENDERER__GRAPHICS__ERROR_HANDLER_HPP
#define ZENDERER__GRAPHICS__ERROR_HANDLER_HPP

#include "Zenderer/Utilities/Assert.hpp"
#include "Zenderer/Graphics/Window.hpp"
#include "Zenderer/Graphics/Font.hpp"

#include "Types.hpp"

namespace zen
{
namespace gfx
{
    static ZEN_API void error_window(const char* msg, const char* title)
    {
        ZEN_ASSERT(msg && title);
        
        gfx::CFont Font;
        
        if(!Font.LoadFromFile(ZENDERER_FONT_PATH"default.ttf"))
        {
            if(!Font.LoadSystemFont())
            {
                error_fallback(msg, title);
            }
        }
        
        std::stringstream err;
        err << "A fatal error occurred:" << std::endl << msg;
        
        uint32_t w = Font.GetTextWidth(err.str());
        uint32_t h = Font.GetTextHeight(err.str());
        
        gfx::CWindow Window(w, h, title);
        
        if(!Window.Init()) error_fallback(msg, title);
        
        Font.Cache(err.str());
        
        while(Window.IsOpen())
        {
            glfwPollEvents();
            Window.Clear();
            Font.DrawCached();
            Window.Update();
        }
        
        exit(1);
    }

}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__ERROR_HANDLER_HPP

/** @} **/
