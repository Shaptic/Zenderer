#ifndef ZENDERER__GUI__FONT_ATLAS_HPP
#define ZENDERER__GUI__FONT_ATLAS_HPP

// Include FreeType2 API.
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "Zenderer/Utilities/Log.hpp"

// Include necessary Zenderer API files for rendering
// bitmaps to entities.
#include "FontCore.hpp"
#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/Graphics/RenderTarget.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gui
{
    using util::LogMode;
    using util::zLog;

    /**
     * A much faster version of the default font class.
     *  The functionality of this class is identical to zen::gui::zFont,
     *  but the internals are much different and the performance
     *  improvement for rendering to an entity is outstanding.
     *
     * @note    This class is fully experimental and untested.
     **/
    class ZEN_API zFont : public asset::zAsset
    {
    public:
        ~zFont(){}

        template<typename T> inline
        zFont& operator<<(const T& data)
        {
            m_str << data;
            return (*this);
        }

        bool LoadFromFile(const string_t& filename)
        {
            const gui::zFontLibrary& Lib = gui::zFontLibrary::InitFreetype();

            if(!Lib.IsInit())
            {
                m_Log   << m_Log.SetSystem("FreeType")
                        << m_Log.SetMode(LogMode::ZEN_FATAL)
                        << "FreeType2 API is not initialized."
                        << zLog::endl;
                return (m_loaded = false);
            }

            if(m_loaded) this->Destroy();

            // Create a new font face.
            if(FT_New_Face(Lib.GetLibrary(), filename.c_str(), 0, &m_FontFace) != 0)
                return (m_loaded = false);

            // Logging.
            m_Log.SetSystem("FreeType");

            // Set the font face size.
            // Since the function expects a size in 1/64 pixels, we multiply
            // by 64 (same as left-shifting 6 bits) before passing.
            // The 96 represents a 96-dpi font bitmap.
            if(FT_Set_Char_Size(m_FontFace, m_size << 6, m_size << 6, 96, 96) != 0)
            {
                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << "Failed to set font size." << zLog::endl;
                return (m_loaded = false);
            }

            // Set universal line height.
            m_height = m_FontFace->height;

            // Calculate the texture atlas size to encompass all 94 chars.
            uint16_t row_w = 0, tmp_w = 0;
            uint16_t atl_h = 10 * m_height;
            uint16_t i = 0;

            char s = ' ', e = '~';
            for( ; s <= e; ++s, ++i)
            {
                if(i % 10 == 0)
                {
                    row_w = math::max<uint16_t>(row_w, tmp_w);
                    tmp_w = 0;
                }

                // Check if the glyph exists for this font.
                uint32_t index = FT_Get_Char_Index(m_FontFace, s);
                if(index == 0) continue;
                tmp_w += this->SizePass(index);
            }

            // We've calculated the total texture atlas size, now
            // we can create it and store the bitmaps inside of it.
            gfxcore::zTexture* tmp = mp_Assets->Create<gfxcore::zTexture>();
            tmp->LoadFromRaw(GL_RGBA8, GL_RGBA, row_w, atl_h, nullptr);
            mp_Atlas->LoadTexture(*tmp);
            mp_Assets->Delete(tmp);

            // Loads all printable ASCII characters.
            s = ' ';
            i = 0;
            uint32_t space = FT_Get_Char_Index(m_FontFace, s);
            math::vector_t Pos;
            for( ; s <= e; ++s, ++i)
            {
                // Check if the glyph exists for this font.
                uint32_t index = FT_Get_Char_Index(m_FontFace, s);

                // Load a glyph only if it exists.
                if(index == 0)
                {
                    m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR) << "Character '" << s
                            << "' does not exist for this font." << zLog::endl;
                    continue;
                }

                glyph_t G = this->LoadGlyphIntoAtlas(s, index, Pos);
                m_glyphData[s] = G;

                Pos.x += G.advance;
                Pos.y += (i % 10) ? m_height : 0;
            }

            m_loaded = true;
            this->SetFilename(filename);

            // Cleanup
            return (m_loaded = FT_Done_Face(m_FontFace) == 0);
        }

        bool Render(obj::zEntity& Ent, const string_t& to_render = "")
        {
            const string_t& text = to_render.empty() ? m_str.str() : to_render;
            if(text.empty() || !m_loaded) return false;

            // Fresh start.
            Ent.Destroy();

            // 4 vertices for each character, and 6 indices for
            // each character.
            uint16_t vlen = text.length() << 2;
            uint16_t ilen = text.length() * 6;

            // Create empty buffers.
            gfxcore::vertex_t* verts = new gfxcore::vertex_t[vlen];
            gfxcore::index_t*  inds  = new gfxcore::index_t [ilen];

            math::vectoru16_t totals;

            // Rendering position. By default, we use the line height, because the any
            // line of the given text will at the very most be as tall as a line. But,
            // it may be true that there are no full line-height characters, so if there
            // are none then we just start at the lowest one.
            math::vector_t Pos(
                0.0,
                math::min<uint16_t>(m_height, this->GetTextHeight(text))
            );

            // Fill up the buffers.
            for(size_t i = 0; i < vlen; i += 4)
            {
                char c = text[i >> 2];

                // Handle newlines by resetting the x-coordinate and
                // increasing the y by the current font faces line height
                // property (universal on all glyphs).
                if(c == '\n')
                {
                    Pos.x  = 0.0;
                    Pos.y += m_height;
                    continue;
                }

                // Use space (' ') as a placeholder for non-renderable chars.
                char letter = (c > '~' || c < ' ') ? ' ' : c;

                // Shortcut to glyph data.
                const glyph_t& gl = m_glyphData[letter];

                real_t gx = gl.position.x;  // Store current x-coordinate.
                real_t gh = gl.position.y;  // Store current y-coordinate.
                Pos.x    += gl.advance;     // Increment position for the next glyph.

                /*
                 * [i]      : top left
                 * [i + 1]  : top right
                 * [i + 2]  : bottom right
                 * [i + 3]  : bottom left
                 */
                verts[i].position   = math::vector_t(Pos.x,             m_height - gx);
                verts[i+1].position = math::vector_t(Pos.x + gl.size.x, m_height - gx);
                verts[i+2].position = math::vector_t(Pos.x + gl.size.x, m_height);
                verts[i+3].position = math::vector_t(Pos.x,             m_height);

                // Load up the bitmap texture coordinates moving
                // counter-clockwise from the origin.
                verts[i].tc     = math::vector_t(0, 1);
                verts[i+1].tc   = math::vector_t(1, 1);
                verts[i+2].tc   = math::vector_t(1, 0);
                verts[i+3].tc   = math::vector_t(0, 0);

                // Uniform font color.
                for(size_t j = i; j < i + 4; ++j) verts[j].color = m_Color;

                // Calculate index starting point. Since 6 indices / vertex,
                // we do i / 4 (since 4 iterations per vertex) to get the
                // vertex number, and * 6 to get to the first index for it.
                int x = (i >> 2) * 6;

                // Standard quad indices.
                inds[x]   = i;
                inds[x+1] = i + 1;
                inds[x+2] = i + 3;
                inds[x+3] = i + 3;
                inds[x+4] = i + 2;
                inds[x+5] = i + 1;

                // Track total dimensions.
                totals.x += math::max<uint16_t>(gl.size.x, gl.advance);
                totals.y  = math::max<uint16_t>(totals.y,  gl.size.y);
            }

            // Render all of the loaded data onto a texture,
            // then assign that texture to the entity.
            gfxcore::DrawBatch D;
            gfxcore::zVertexArray VAO(GL_STATIC_DRAW);
            gfx::zRenderTarget FBO(totals.x, totals.y);

            D.Vertices = verts;
            D.vcount   = vlen;
            D.Indices  = inds;
            D.icount   = ilen;

            if(!(VAO.Init() && FBO.Init()))
            {
                delete[] verts;
                delete[] inds;
                return false;
            }

            VAO.AddData(D);
            VAO.Offload();

            VAO.Bind(); FBO.Bind();
            FBO.Clear();

            bool blend = gfxcore::zRenderer::BlendOperation(
                                    gfxcore::BlendFunc::IS_ENABLED);

            gfxcore::zRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);
            mp_Atlas->Enable();

            for(size_t i = 0, j = text.length(); i < j; ++i)
            {
                // Render each character (skip if unrenderable).
                if(text[i] > '~' || text[i] < ' ') continue;

                const glyph_t& g = m_glyphData[text[i]];
                const int offset[] = { g.position.x, g.position.y };
                mp_Atlas->GetEffect().SetParameter("offset", offset, 2);
                GL(glDrawElements(GL_TRIANGLES, 6, gfxcore::INDEX_TYPE,
                                 (void*)(sizeof(gfxcore::index_t) * i * 6)));
            }

            FBO.Unbind();
            mp_Atlas->Disable();

            // Only disable blending if it wasn't enabled prior to calling
            // this method (checked above).
            if(!blend) gfxcore::zRenderer::BlendOperation(
                                  gfxcore::BlendFunc::DISABLE_BLEND);

            // Now the string has been rendered to the FBO texture, so all we need to
            // do is create a material and attach it to the quad.

            // Create a texture wrapper from the texture handle in the FBO.
            gfxcore::zTexture* pTexture =
                mp_Assets->Create<gfxcore::zTexture>(this->GetOwner());
            pTexture->LoadFromExisting(FBO.GetTexture());

            // Retrieve the raw data.
            const unsigned char* data =
                reinterpret_cast<const unsigned char*>(pTexture->GetData());

            // Load the texture wrapper with raw data, because the FBO will go out
            // of scope soon, thus destroying the texture handle, but the data needs
            // to be preserved.
            pTexture->LoadFromRaw(GL_RGBA8, GL_RGBA, totals.x, totals.y, data);

            gfx::zMaterial M(*mp_Assets);
            if(!M.LoadEffect(gfx::EffectType::GRAYSCALE) ||
               !M.LoadTexture(*pTexture))
            {
                delete[] data;
                delete[] verts;
                delete[] inds;

                FBO.Destroy() && VAO.Destroy();
                return false;
            }

            gfx::zQuad Q(*mp_Assets, totals.x, totals.y);
            Q.AttachMaterial(M);
            Q.Create();
            Q.SetColor(color4f_t());

            Ent.AddPrimitive(Q);

            mp_Assets->Delete(pTexture);

            delete[] data;
            delete[] verts;
            delete[] inds;

            return FBO.Destroy() && VAO.Destroy();
        }

        void SetColor(const color4f_t& Color) { m_Color = Color; }
        void SetSize(const uint16_t size) { m_size = size; }
        void SetStacking(const bool flag) {}

        void ClearString()
        {
            m_str.str(std::string());
        }

        uint16_t GetTextWidth(const string_t& text) const
        {
            if(text.empty()) return 0;

            uint16_t w = 0, tmp_w = 0;
            uint16_t l = text.length();

            for(size_t i = 0; i < l; ++i)
            {
                if(text[i] == '\n')
                {
                    w = math::max<uint16_t>(w, tmp_w);
                    tmp_w = 0;
                }
                else
                {
                    const auto it = m_glyphData.find(text[i]);
                    if(it != m_glyphData.end())
                    {
                        tmp_w += math::max<uint16_t>(it->second.size.x,
                                                     it->second.advance);
                    }
                }
            }

            return math::max<uint16_t>(w, tmp_w);
        }

        uint16_t GetTextHeight(const string_t& text) const
        {
            if(text.empty()) return 0;

            uint16_t h = 0, tmp_h = 0;
            for(auto& i : text)
            {
                if(i == '\n')
                {
                    h = math::max(h, tmp_h);
                    tmp_h = 0;
                }
                else
                {
                    const auto g = m_glyphData.find(i);
                    tmp_h = math::max<uint16_t>(
                        g->second.size.y + g->second.position.y, tmp_h);
                }
            }

            return math::max(h, tmp_h);
        }

        const void* const GetData() const
        {
            return reinterpret_cast<const void* const>(&m_glyphData);
        }

        uint16_t GetLineHeight() const { return m_height; }

        void AttachManager(asset::zAssetManager& Mgr)
        {
            mp_Assets = &Mgr;
            if(mp_Atlas != nullptr)
                delete mp_Atlas;

            mp_Atlas = new gfx::zMaterial(Mgr);
            mp_Atlas->LoadEffect(gfx::EffectType::SPRITESHEET);
            gfx::zEffect& E = mp_Atlas->GetEffect();
            E.Enable();
            E.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
            E.SetParameter("proj", gfxcore::zRenderer::GetProjectionMatrix());
            E.Disable();
        }

        friend class ZEN_API asset::zAssetManager;

    private:
        zFont(const void* const owner = nullptr) : zAsset(owner),
            mp_Atlas(nullptr)
        {
        }

        bool Destroy()
        {
            if(m_loaded && mp_Atlas)
            {
                delete mp_Atlas;
                m_loaded = (mp_Atlas = nullptr) != nullptr;
            }

            return !m_loaded;
        }

        uint16_t SizePass(const uint16_t index)
        {
            FT_Glyph g;

            // Render as a monochrome bitmap into glyph struct.
            FT_Load_Glyph(m_FontFace, index, FT_LOAD_RENDER);
            FT_Render_Glyph(m_FontFace->glyph, FT_RENDER_MODE_NORMAL);
            FT_Get_Glyph(m_FontFace->glyph, &g);

            return m_FontFace->glyph->bitmap.width;
        }

        glyph_t LoadGlyphIntoAtlas(const char c, const uint16_t index, const math::vector_t& Pos)
        {
            FT_Glyph g;

            // Render as a monochrome bitmap into glyph struct.
            FT_Load_Glyph(m_FontFace, index, FT_LOAD_RENDER);
            FT_Render_Glyph(m_FontFace->glyph, FT_RENDER_MODE_NORMAL);
            FT_Get_Glyph(m_FontFace->glyph, &g);

            // Minor shortcuts.
            FT_GlyphSlot slot = m_FontFace->glyph;
            FT_Bitmap& bitmap = slot->bitmap;

            // Shortcut to dimensions.
            uint32_t w = bitmap.width;
            uint32_t h = bitmap.rows;

            if(w == 0 || h == 0)
            {
                m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                        << "Empty character bitmap for '" << c << '\'' << zLog::endl;
            }

            // Create the OpenGL texture and store the FreeType bitmap
            // in it as a monochrome texture.

            // Store for restoring later.
            GLint pack;
            GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &pack));
            GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

            GL(glTexSubImage2D(GL_TEXTURE_2D, 0, Pos.x, Pos.y,
                               w, h, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.buffer));

            GL(glPixelStorei(GL_UNPACK_ALIGNMENT, pack));

            glyph_t gl;
            gl.texture  = nullptr;
            gl.size     = math::vectoru16_t(w, h);
            gl.position = math::vectoru16_t(slot->metrics.horiBearingY >> 6,
                                            slot->metrics.horiBearingX >> 6);
            gl.advance  = slot->advance.x >> 6;

            // Clean up TTF data.
            FT_Done_Glyph(g);

            return gl;
        }

        asset::zAssetManager*   mp_Assets;
        gfx::zMaterial*         mp_Atlas;

        color4f_t m_Color;
        FT_Face m_FontFace;

        std::map<char, glyph_t> m_glyphData;
        std::stringstream m_str;

        uint16_t m_size;
        uint16_t m_height;
    };
}
}

#endif // ZENDERER__GUI__FONT_ATLAS_HPP
