/**
 * @file
 *  Zenderer/GUI/Font.hpp - A wrapper for TrueType font loading and rendering.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.0
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

#ifndef ZENDERER__GUI__FONT_HPP
#define ZENDERER__GUI__FONT_HPP

// Include FreeType2 API.
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "Zenderer/Utilities/Log.hpp"

// Include necessary Zenderer API files for rendering
// bitmaps to entities.
#include "FontCore.hpp"
#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gui
{
    class ZEN_API CFont : public asset::CAsset
    {
    public:
        CFont(const void* const owner = nullptr);
        ~CFont();
        
         /**
         * Loads a TrueType font from disk.
         *  Here, all of the printable characters in the ASCII table
         *  are loaded as bitmaps, then are turned into render-able
         *  textures in GPU memory. Their dimensions are stored for
         *  rendering later on.
         *  Since this inherits from zen::asset::CAsset, it's 
         *  impossible to specify font size when using an asset manager
         *  and `Create<>()`'ing from a filename. Thus the recommended
         *  approach for this is `Create<>()`'ing without a filename
         *  (thus creating a raw asset), and then calling SetSize() 
         *  prior to a LoadFromFile() call.
         *
         * @param   filename    Font filename
         *
         * @pre     CFontLibrary::Init() must have been called.
         *
         * @return  `true`  if everything went smoothly,
         *          `false` otherwise.
         *
         * @see     _fonts-examples.html
         **/
        bool LoadFromFile(const string_t& filename);
        
        /**
         * Loads a font from an existing instance.
         *  There is no way (without enabling RTTI) to check if the given
         *  parameter is indeed a valid CFont instance, therefore this is
         *  dependant on the user of this API.
         *  This creates a deep copy of internal glyph texture data and
         *  all relevent asset metadata.
         *
         * @param   pCopy   Font asset to copy from.
         **/
        bool LoadFromExisting(const asset::CAsset* pCopy);
         
        /// Returns raw font glyph bitmap data.
        const void* const GetData() const;
        
        /// @todo   Store line height properly.
        void Render(obj::CEntity& Ent, const string_t& text);
        
    private:
        bool Destroy();        
        bool LoadGlyph(const char c, const uint32_t index);

        color4f_t m_Color;
        FT_Face m_FontFace;
        std::map<char, glyph_t> mp_glyphData;
        uint16_t m_size;
    };
    
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__FONT_HPP

/** @} **/

/**
 * @class zen::gfx::CFont
 * @description
 *
 *
 * @example Fonts
 * @subsection Standard Font Loading
 *  In this example we will demonstrate the standard procedure for
 *  loading a font and rendering some string to the screen.
 *  First, we assume that a valid context has been created and that
 *  there is an existing asset manager we wish to use, referred to 
 *  as `Win` and `Assets` below, respectively. The default font size 
 *  used when creating from an asset manager is 18.
 *
 *  @code
 *  gfxcore::CFont* Font = Assets.Create<gfxcore::CFont>(
 *      string_t("default.ttf"));
 *
 *  // Error checking omitted for brevity.
 *
 *  // Render a string to an entity
 *  obj::CEntity& Score = Scene.AddEntity();
 *  Font->Render(Score, "Score: 0");
 *
 *  // Move it to the top of the screen
 *  Score.Move(0.0, Score.GetH());
 *
 *  // Set a high depth so it's always visible, in case of various
 *  // effects (like shadows) that occlude based on depth.
 *  Score.SetDepth(69);
 *
 *  // Now it will be rendered in the scene whenever 
 *  // gfx::CScene::Draw() is called.
 *
 *  // Clean up, optional.
 *  Assets.Delete(Font);  
 *  @endcode
 *
 *  Since scenes are designed to manage entities independently, the
 *  `CFont::Render()` method cannot simply return a `obj::CEntity` 
 *  instance to the user, since then, when using a scene, it would 
 *  not know if it was `delete`-able or not. Thus, even when not 
 *  using a scene, fonts must be given an existing `obj::CEntity`
 *  instance to work on. This is demonstrated in the following
 *  example.
 *  This example also demonstrates the technique for loading a font
 *  and specifying the size, since this cannot be done without a call
 *  to `CFont::Resize()` when using the manager.
 *
 *  @code
 *  gfxcore::CFont* Font = Assets.Create<gfxcore::CFont>();
 *  Font->LoadFromFile("default.ttf", 8);
 *
 *  // Render a string to an entity without using a scene.
 *  obj::CEntity Score;
 *  Font->Render(Score, "Score: 0");
 *
 *  // Move it to the top of the screen
 *  Score.Move(0.0, Score.GetH());
 *
 *  // Set a high depth so it's always visible, in case of various
 *  // effects (like shadows) that occlude based on depth.
 *  Score.SetDepth(69);
 *
 *  // Later, in the game loop
 *  while(!Win.IsOpen())
 *  {
 *      // Timing, events, logic, etc.
 *      // ...
 *
 *      Win.Clear();
 *      Scene.Draw();
 *      Score.Draw();
 *      Win.Update();
 *
 *      // Post-render operations, etc.
 *      // ...
 *  }
 *
 *  // Clean up, optional.
 *  Assets.Delete(Font);  
 *  @endcode 
 **/
