#ifndef ZENDERER__GUI__FONT_CORE_HPP
#define ZENDERER__GUI__FONT_CORE_HPP

// Include FreeType2 API.
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "FontCore.hpp"

// Include necessary Zenderer API files for rendering
// bitmaps to entities.
#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/CoreGraphics/Texture.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gui
{
    /// Initializes the core FreeType2 library.
    class ZEN_API CFontLibrary : CSubsystem
    {
    public:
        ~CFontLibrary();
        
        /**
         * Initializes the FreeType library.
         *  This should be called prior to ANY other font operations,
         *  preferably by the @a Zenderer initialization function.
         *
         * @return  `true` on success, `false` on failure.
         *
         * @see     zen::Init()
         *
         * @note    Initialization is guaranteed to only occur one 
         *          time despite multiple potential calls to this
         *          method.
         **/
        bool Init()
        {
            if(m_init) return true;            
            return (m_init = FT_Init_FreeType(&m_Library));
        }
        
        bool Destroy()
        {
            if(!m_init) return true;
            FT_Done_FreeType(m_Library);
            return !(m_init = false);
        }
        
        static CFontLibrary& InitFreetype()
        {
            static CFontLibrary s_TTF();
            s_TTF.Init();
            return s_TTF;
        }
        
        FT_Library& GetLibrary();
        
    private:
        FT_Library m_Library;
    };
    
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GUI__FONT_CORE_HPP

/** @} **/
