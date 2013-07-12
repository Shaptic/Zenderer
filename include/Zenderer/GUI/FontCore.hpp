/**
 * @file
 *  Zenderer/GUI/FontCore.hpp - Core library wrapper for FreeType 2.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup GUI
 * @{
 **/

#ifndef ZENDERER__GUI__FONT_CORE_HPP
#define ZENDERER__GUI__FONT_CORE_HPP

// Include FreeType2 API.
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "Zenderer/Core/Subsystem.hpp"
#include "Zenderer/CoreGraphics/Texture.hpp"
#include "Zenderer/Math/Shapes.hpp"

namespace zen
{
    
/// A collection of TrueType font rendering wrapper utilities.
namespace gui
{
    /// Initializes the core FreeType2 library.
    class ZEN_API CFontLibrary : public CSubsystem
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
            static CFontLibrary s_TTF;
            s_TTF.Init();
            return s_TTF;
        }
        
        FT_Library& GetLibrary() const;
        
    private:
        FT_Library m_Library;
    };
    
    struct glyph_t
    {
        gfxcore::CTexture* texture;
        math::rect_t dim;
    };
    
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GUI__FONT_CORE_HPP

/** @} **/
