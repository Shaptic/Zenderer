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
#include "FontCore.hpp"

// Include necessary Zenderer API files for rendering
// bitmaps to entities.
#include "Zenderer/CoreGraphics/OpenGL.hpp"
#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/CoreGraphics/Texture.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gui
{
    class ZEN_API CFont : public asset::CAsset
    {
    public:
        CFont(const void* const owner) : CAsset(owner), m_size(18) {}
        ~CFont()
        {
            this->Destroy();
        }
        
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
        bool LoadFromFile(const string_t& filename)
        {
            CFontLibrary& Lib = CFontLibrary.GetTTFLibrary();
            
            if(!Lib.IsInit())
            {
                m_Log   << m_Log.SetSystem("FreeType")
                        << m_Log.SetMode(LogMode::ZEN_FATAL)
                        << "FreeType2 API is not initialized."
                        << CLog::endl;
                return (m_loaded = false);
            }
            
            if(m_loaded) this->Destroy();
            
            // Create a new font face.
            if(FT_New_Face(Lib, filename.c_str(), 0, m_FontFace) != 0)
                return (m_loaded = false);
            
            // Logging.
            m_Log.SetSystem("FreeType");
            
            // Set the font face size.
            // Since the function expects a size in 1/64 pixels, we multiply
            // by 64 (same as left-shifting 6 bits) before passing.
            // The 96 represents a 96-dpi font bitmap.
            if(FT_Set_Char_Size(m_FontFace, size << 6, size << 6, 96, 96) != 0)
            {
                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << "Failed to set font size." << CLog::endl;
                return (m_loaded = false);
            }
            
            // Loads all printable ASCII characters.
            mp_glyphData.reserve('~' - ' ');
            uint32_t space = FT_Get_Char_Index(m_FontFace, ' ');
            
            for(size_t s = ' ', e = '~'; s <= e; ++s)
            {
                // Check if the glyph exists for this font.
                uint32_t index = FT_Get_Char_Index(m_FontFace, s);
                
                // Load a glyph if it exists, and a space (' ') placeholder
                // otherwise.
                if(index == 0)
                {
                    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                            << "Character '" << s
                            << "' does not exist for this font." << CLog::endl;
                }
                
                this->LoadGlyph(s, (!index) ? space : index);
            }
            
            m_loaded = true;
            this->SetFilename(filename);
            
            // Cleanup
            return (m_loaded = FT_Done_Face(m_FontFace) == 0);
        }
        
        
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
        const void* const GetData() const
        {
            return static_cast<const void* const>(mp_glyphData.data());
        }
        
        /// @todo   Store line height properly.
        void Render(obj::CEntity& Ent, const string_t& text)
        {
            ZEN_ASSERT(!text.empty());
            
            if(!m_loaded) return;
            
            // Fresh start.
            Ent.Destroy();
            
            // 4 vertices for each character, and 6 indices for
            // each character.
            uint16_t vlen = text.length() << 2;
            uint16_t ilen = text.length() * 6;
            
            // Create empty buffers.
            gfxcore::vertex_t* verts = new gfxcore::vertex_t[vlen];
            gfxcore::index_t*  inds  = new gfxcore::index_t [ilen];
            
            uint16_t w = 0, h = 0;
            
            // Coordinates to start rendering at.
            int32_t last_w = 0, last_h = 0;
            
            // Fill up the buffers.
            for(size_t i = 0; i < vlen; i += 4)
            {
                char c = text[i >> 2];
                
                // Just increase Y-coordinate for rendering.
                // We use an arbitrary "large" letter that takes
                // up an entire line to adjust properly.
                if(c == '\n')
                {
                    last_w = 0;
                    last_h += mp_glyphData['H'].dim.y + mp_glyphData['H'].dim.h;
                    continue;
                }
                
                // Use space (' ') as a placeholder for non-renderable chars.
                char letter = (c > '~' || c < ' ') ? ' ' : c;
                
                // Shortcut to glyph dimensions.
                const rect_t& Dim = mp_glyphData[letter].dim;
                
                real_t w = last_w;  // Store current x-coordinate.
                real_t h = Dim.y;   // Store current y-coordinate.
                last_w  += Dim.w;   // Increment for next glyph.

                // [i]      : top left
                // [i + 1]  : top right
                // [i + 2]  : bottom right
                // [i + 3]  : bottom left
                verts[i].position   = math::vector_t(w,     last_h - Dim.h);
                verts[i+1].position = math::vector_t(last_w,last_h - Dim.h);
                verts[i+2].position = math::vector_t(last_w,last_h - Dim.h + h);
                verts[i+3].position = math::vector_t(w,     last_h - Dim.h + h);
                
                // Load up the bitmap texture coordinates moving
                // counter-clockwise from the origin.
                verts[i].tc     = math::vector_t(0, 0);
                verts[i+1].tc   = math::vector_t(1, 0);
                verts[i+2].tc   = math::vector_t(1, 1);
                verts[i+3].tc   = math::vector_t(0, 1);
                
                // Uniform font color.
                for(size_t j = i; j < i + 4; ++j)
                    verts[j].color = m_Color;
                    
                int x = (i >> 2) * 6;
                
                // Standard quad indices.
                inds[x]   = i;
                inds[x+1] = i + 1;
                inds[x+2] = i + 3;
                inds[x+3] = i + 3;
                inds[x+4] = i + 2;
                inds[x+5] = i + 1;
                
                // Track total dimensions.
                max_w += Dim.w;
                max_h  = math::max<uint16_t>(max_h, Dim.h + h);
            }
            
            // Render all of the loaded data onto a texture,
            // then assign that texture to the entity.
            gfxcore::DrawBatch D;
            gfxcore::CVertexArray VAO(GL_STATIC_DRAW);
            gfxcore::CFrameBuffer FBO(max_w, max_h);
            
            D.Vertices = verts;
            D.vcount   = vlen;
            D.Indices  = inds;
            D.icount   = ilen;
            
            VAO.Init(); FBO.Init();
            VAO.AddData(D);
            VAO.Offload();
            
            VAO.Bind(); FBO.Bind();
            FBO.Clear();
            
            gfxcore::CRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);
            
            for(size_t i = 0, j = text.length(); i < j; ++i)
            {
                // Render each character (skip if unrenderable).
                if(text[i] > '~' || text[i] < ' ') continue;
                
                mp_glyphTextures[text[i]].pTexture->Bind();
                GL(glDrawElements(GL_TRIANGLES, 6, gfxcore::INDEX_TYPE,
                    (void*)(sizeof(gfxcore::index_t) * i * 6)));
            }

            gfxcore::CRenderer::ResetMaterialState();
            gfxcore::CRenderer::BlendOperation(gfxcore::BlendFunc::DISABLE_BLEND);
            
            // Now the string has been rendered to the FBO texture,
            // so all we need to do is create a material and attach 
            // it to the quad.
            gfx::CMaterial* M = new gfx::CMaterial(m_Assets);
            M->LoadEffect(gfx::EffectType::GRAYSCALE);
            M->LoadTextureFromHandle(FBO.GetTexture());
            
            gfx::CQuad Q(max_w, max_h);
            Q.AttachMaterial(M);
            Q.Create();
            
            Ent.LoadFromPrimitive(Q);
        }
        
    private:
        bool Destroy();
        
        bool LoadGlyph(const char c, const uint32_t index)
        {
            FT_Glyph g;
            
            // Render as a monochrome bitmap into glyph struct.
            FT_Load_Glyph(m_FontFace, index, FT_LOAD_RENDER);
            FT_Render_Glyph(m_FontFace->glyph, FT_RENDER_MODE_NORMAL);
            FT_Get_Glyph(m_FontFace->glyph, &g);
            
            // Minor shortcuts.
            FT_GlyphSlot slot = m_FontFace->glyph;
            FT_Bitmap& bitmap  = slot->bitmap;
            
            // Shortcut to dimensions.
            uint32_t w = bitmap.width;
            uint32_t h = bitmap.height;
            
            // Create the OpenGL texture and store the FreeType bitmap
            // in it as a monochrome texture.
            
            // Store for restoring later.
            GLint pack;
            GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &pack));
            GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1))

            gfx::CTexture* pTexture = m_Assets.Create<gfx::CTexture*>(mp_owner);
            pTexture->LoadFromRaw(GL_R8, GL_RED, w, h, bitmap.buffer);
            
            GL(glPixelStorei(GL_UNPACK_ALIGNMENT, pack));
            
            // Clean up TTF data.
            FT_Done_Glyph(g);
            
            // Store the glyph internally.
            glyph_t glyph;
            glyph.texture   = pTexture;
            glyph.dim       = math::rect_t(w, h,    // Raw bitmap w/h        
                slot->advance.x << 6,               // Pixels to adjust till next character
                slot->metrics.horiBearingY << 6);   // Line height offset (stuff like 'y' and 'h')
            
            mp_glyphData.push_back(glyph);
            return true;
        }

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
