#include "Zenderer/GUI/Font.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using gui::zFont;

zFont::zFont(const void* const owner,
             const void* const settings) :
    zAsset(owner), m_Color(0.0, 0.0, 0.0, 1.0),
    m_size(18), m_height(0), m_stack(false),
    mp_FontFx(nullptr)
{
    if(settings != nullptr)
    {
        const fontcfg_t* const cfg = static_cast<const fontcfg_t* const>(
            settings);

        m_size = cfg->size;
    }
}

zFont::~zFont()
{
    this->Destroy();
}

bool zFont::LoadFromFile(const string_t& filename)
{
    ZEN_ASSERT(mp_Parent != nullptr);
    if(mp_FontFx == nullptr)
    {
        mp_FontFx = new gfx::zEffect(gfx::EffectType::ZFONT, *mp_Parent);
        if(!mp_FontFx->Init())
        {
            delete mp_FontFx;
            mp_FontFx = nullptr;

            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << "Internal font rendering effect not loaded. "
                    << "Check the log for error details." << zLog::endl;
            return (m_loaded = false);
        }
    }

    // Logging.
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO) << m_Log.SetSystem("FreeType")
            << "Loading font '" << filename << "'." << zLog::endl;

    const gui::zFontLibrary& Lib = gui::zFontLibrary::InitFreetype();

    if(!Lib.IsInit())
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL)
                << "FreeType2 API is not initialized." << zLog::endl;
        return (m_loaded = false);
    }

    if(m_loaded) this->Destroy();

    // Create a new font face.
    if(FT_New_Face(Lib.GetLibrary(), filename.c_str(), 0, &m_FontFace) != 0)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << "Failed to load '" << filename << "'." << zLog::endl;

        return (m_loaded = false);
    }

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
    m_height = (m_FontFace->ascender + m_FontFace->descender) >> 6;

    // Loads all printable ASCII characters.
    uint32_t space = FT_Get_Char_Index(m_FontFace, ' ');
    for(size_t s = ' ', e = '~'; s <= e; ++s)
    {
        // Check if the glyph exists for this font.
        uint32_t index = FT_Get_Char_Index(m_FontFace, s);

        // Load a glyph if it exists, and a space (' ') placeholder
        // otherwise.
        if(index == 0)
        {
            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR) << "Character '" << s
                    << "' does not exist for this font." << zLog::endl;
        }

        this->LoadGlyph(s, (!index) ? space : index);
    }

    this->SetFilename(filename);

    // Cleanup
    return (m_loaded = FT_Done_Face(m_FontFace) == 0);
}

const void* const zFont::GetData() const
{
    return reinterpret_cast<const void* const>(&m_glyphData);
}

bool zFont::Render(obj::zEntity& Ent, const string_t& to_render) const
{
    gfxcore::zTexture& Texture = mp_Parent->Create<gfxcore::zTexture>();
    if(!this->Render(Texture, to_render))
    {
        mp_Parent->Delete(&Texture);
        return false;
    }

    gfx::zMaterial M(*mp_Parent);
    if(!M.LoadTexture(Texture))
    {
        mp_Parent->Delete(&Texture);
        return false;
    }

    gfx::zQuad Q(*mp_Parent, Texture.GetWidth(), Texture.GetHeight());
    Q.AttachMaterial(M);
    Q.SetColor(color4f_t());
    Q.FlipOn(gfx::Axis::Y);
    Q.Create();

    if(!m_stack) Ent.Destroy();
    Ent.AddPrimitive(std::move(Q));
    mp_Parent->Delete(&Texture);
    return true;
}

bool zFont::Render(gfxcore::zTexture& Texture, const string_t& to_render) const
{
    ZEN_ASSERT(mp_Parent != nullptr);
    const string_t& text = to_render.empty() ? m_str.str() : to_render;

    if(text.empty() || !m_loaded || mp_FontFx == nullptr) return false;

    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG) << m_Log.SetSystem("FreeType")
            << "Rendering text string '" << text << "'." << zLog::endl;

    // 4 vertices for each character, and 6 indices for
    // each character.
    uint16_t vlen = text.length() << 2;
    uint16_t ilen = text.length() * 6;

    // Create empty buffers.
    gfxcore::vertex_t* verts = new gfxcore::vertex_t[vlen];
    gfxcore::index_t*  inds  = new gfxcore::index_t [ilen];

    glm::u16vec2 totals;
    uint16_t tmp_tx = 0;

    // Rendering position. We calculate this based on the maximum y-bearing
    // in the first line of text. Thus, we find the first \n character, if
    // any, we stop max()-ing the bearing.
    // Thus the (0, 0, 1) coordinate is the top of the line. Any subsequent
    // lines just add their height to the y coordinate and keep rendering.
    math::zVector<int16_t> Pos;

    for(auto& i : text)
    {
        if(i == '\n') break;

        const auto it = m_glyphData.find(i);
        if(it == m_glyphData.end()) continue;
        Pos.y = math::max<int16_t>(Pos.y, it->second.position.y);
    }
    Pos.y = -Pos.y;

    // Fill up the buffers.
    for(size_t i = 0; i < vlen; i += 4)
    {
        char c = text[i >> 2];

        // Handle newlines by resetting the x-coordinate and
        // increasing the y by the current font faces line height
        // property (universal on all glyphs).
        if(c == '\n')
        {
            Pos.x  = 0; Pos.y -= m_height;
            totals.x = math::max(tmp_tx, totals.x);
            tmp_tx = 0;
            continue;
        }

        // Use space (' ') as a placeholder for non-renderable chars.
        char letter = (c > '~' || c < ' ') ? ' ' : c;

        // Shortcut to glyph data.
        const auto it = m_glyphData.find(letter);

        // If the character doesn't exist, we just skip it.
        if(it == m_glyphData.end() || it->first == ' ')
        {
            // Increment position for the next glyph.
            tmp_tx  += m_glyphData.find(' ')->second.advance;
            Pos.x   += m_glyphData.find(' ')->second.advance;
            continue;
        }

        const glyph_t& gl = it->second;

        /*
         * [i]      : top left
         * [i + 1]  : top right
         * [i + 2]  : bottom right
         * [i + 3]  : bottom left
         */
        uint16_t start_y    = std::abs(gl.position.y + Pos.y);
        verts[i].position   = glm::vec2(Pos.x,           start_y);
        verts[i+1].position = glm::vec2(Pos.x+gl.size.x, start_y);
        verts[i+2].position = glm::vec2(Pos.x+gl.size.x, start_y+gl.size.y);
        verts[i+3].position = glm::vec2(Pos.x,           start_y+gl.size.y);

        // The glyph textures are stored upside-down, so we set the texture
        // coordinates to flip them back appropriately.
        verts[i].tc     = glm::vec2(0, 0);
        verts[i+1].tc   = glm::vec2(1, 0);
        verts[i+2].tc   = glm::vec2(1, 1);
        verts[i+3].tc   = glm::vec2(0, 1);

        // Uniform font color.
        for(size_t j = i; j < i + 4; ++j) verts[j].color = m_Color;

        // Calculate index starting point. Since 6 indices per vertex,
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
        tmp_tx   += math::max(gl.size.x, gl.advance);
        totals.y  = math::max<uint16_t>(totals.y, start_y + gl.size.y);

        // Increment position for the next glyph.
        Pos.x += gl.advance;
    }

    totals.x = math::max(tmp_tx, totals.x);

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

    VAO.Bind();
    FBO.Bind();

    mp_FontFx->Enable();

    bool blend = gfxcore::zRenderer::BlendOperation(
        gfxcore::BlendFunc::IS_ENABLED);

    gfxcore::zRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);

    for(size_t i = 0, j = text.length(); i < j; ++i)
    {
        // Render each character (skip if unrenderable or no effect).
        if(text[i] > '~' || text[i] <= ' ') continue;

        const glyph_t& g = m_glyphData.find(text[i]) == m_glyphData.end()   ?
                           m_glyphData.find('?')->second                    :
                           m_glyphData.find(text[i])->second;

        g.texture->Bind();
        GL(glDrawElements(GL_TRIANGLES, 6, gfxcore::INDEX_TYPE,
            (void*)(sizeof(gfxcore::index_t) * i * 6)));
    }

    FBO.Unbind();
    gfxcore::zRenderer::ResetMaterialState();

    // Only disable blending if it wasn't enabled prior to calling
    // this method (checked above).
    if(!blend) gfxcore::zRenderer::BlendOperation(
                          gfxcore::BlendFunc::DISABLE_BLEND);

    // Now the string has been rendered to the FBO texture, so all we need to
    // do is create a material and attach it to the quad.

    // Create a texture wrapper from the texture handle in the FBO.
    Texture.CopyFromExisting(FBO.GetTexture());

    delete[] verts;
    delete[] inds;

    return FBO.Destroy() && VAO.Destroy();
}

void zFont::ClearString()
{
    m_str.str(std::string());
}

bool zFont::Destroy()
{
    for(auto& i : m_glyphData)
    {
        mp_Parent->Delete(i.second.texture);
        i.second.texture = nullptr;
    }

    m_glyphData.clear();
    this->ClearString();
    return true;
}

bool zFont::LoadGlyph(const char c, const uint16_t index)
{
    ZEN_ASSERT(mp_Parent != nullptr);
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
                << "No character bitmap for '" << c << '\'' << zLog::endl;
    }

    // Create the OpenGL texture and store the FreeType bitmap
    // in it as a monochrome texture.

    // Store for restoring later.
    GLint pack;
    GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &pack));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    gfxcore::zTexture& Texture = mp_Parent->Create<gfxcore::zTexture>(this->GetOwner());
    Texture.LoadFromRaw(GL_R8, GL_RED, w, h, bitmap.buffer);
    std::stringstream ss;
    ss << 's' << m_size << " bitmap for '" << c
       << "'; " << util::string_hash(m_filename);
    Texture.SetFilename(ss.str());

    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, pack));

    // Store the glyph internally.
    glyph_t glyph;
    glyph.texture   = &Texture;
    glyph.size      = math::zVector<uint32_t>(w, h);
    glyph.position  = math::zVector<int32_t>(slot->metrics.horiBearingX >> 6,
                                             slot->metrics.horiBearingY >> 6);
    glyph.advance   = slot->advance.x >> 6;
    m_glyphData[c]  = std::move(glyph);

    // Clean up TTF data.
    FT_Done_Glyph(g);

    return true;
}

void zFont::SetColor(const color4f_t& Color)
{
    m_Color = Color;
}

void zFont::SetColor(const real_t r, const real_t g, const real_t b)
{
    m_Color.r = r;
    m_Color.g = g;
    m_Color.b = b;
}

void zFont::SetStacking(const bool flag)
{
    m_stack = flag;
}

uint16_t zFont::GetTextWidth(const string_t& text) const
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

uint16_t zFont::GetTextHeight(const string_t& text) const
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
