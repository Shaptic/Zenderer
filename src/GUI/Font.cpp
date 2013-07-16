#include "Zenderer/GUI/Font.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gui::CFont;

CFont::CFont(const void* const owner) :
    CAsset(owner), mp_Assets(nullptr), m_Color(0.0, 0.0, 0.0, 1.0),
    m_size(18), m_height(0) {}

CFont::~CFont()
{
    this->Destroy();
}

bool CFont::LoadFromFile(const string_t& filename)
{
    const gui::CFontLibrary& Lib = gui::CFontLibrary::InitFreetype();

    if(!Lib.IsInit())
    {
        m_Log   << m_Log.SetSystem("FreeType")
                << m_Log.SetMode(LogMode::ZEN_FATAL)
                << "FreeType2 API is not initialized." << CLog::endl;
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
                << "Failed to set font size." << CLog::endl;
        return (m_loaded = false);
    }
    
    // Set universal line height.
    m_height = m_FontFace->height;

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
                    << "' does not exist for this font." << CLog::endl;
        }

        this->LoadGlyph(s, (!index) ? space : index);
    }

    this->SetFilename(filename);

    // Cleanup
    return (m_loaded = FT_Done_Face(m_FontFace) == 0);
}

const void* const CFont::GetData() const
{
    return reinterpret_cast<const void* const>(&m_glyphData);
}

bool CFont::Render(obj::CEntity& Ent, const string_t to_render)
{
    ZEN_ASSERTM(mp_Assets != nullptr, "an asset manager must be attached");
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

        /*
         * [i]      : top left
         * [i + 1]  : top right
         * [i + 2]  : bottom right
         * [i + 3]  : bottom left
         */
        verts[i].position   = math::vector_t(Pos.x,             m_height - gl.position.x);
        verts[i+1].position = math::vector_t(Pos.x + gl.size.x, m_height - gl.position.x);
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
        totals.y  = math::max<uint16_t>(totals.y, gl.size.y);

        // Increment position for the next glyph.
        Pos.x += gl.advance;
    }

    // Render all of the loaded data onto a texture,
    // then assign that texture to the entity.
    gfxcore::DrawBatch D;
    gfxcore::CVertexArray VAO(GL_STATIC_DRAW);
    gfx::CRenderTarget FBO(totals.x, totals.y);

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

    bool blend = gfxcore::CRenderer::BlendOperation(
                            gfxcore::BlendFunc::IS_ENABLED);
    
    gfxcore::CRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);
    gfxcore::CRenderer::GetDefaultEffect().Enable();
    gfxcore::CRenderer::GetDefaultEffect().SetParameter("proj",
               gfxcore::CRenderer::GetProjectionMatrix());

    for(size_t i = 0, j = text.length(); i < j; ++i)
    {
        // Render each character (skip if unrenderable).
        if(text[i] > '~' || text[i] < ' ') continue;

        m_glyphData[text[i]].texture->Bind();
        GL(glDrawElements(GL_TRIANGLES, 6, gfxcore::INDEX_TYPE,
            (void*)(sizeof(gfxcore::index_t) * i * 6)));
    }

    FBO.Unbind();
    gfxcore::CRenderer::GetDefaultEffect().SetParameter("proj",
               gfxcore::CRenderer::GetProjectionMatrix());
    gfxcore::CRenderer::ResetMaterialState();
    
    // Only disable blending if it wasn't enabled prior to calling
    // this method (checked above).
    if(!blend) gfxcore::CRenderer::BlendOperation(
                          gfxcore::BlendFunc::DISABLE_BLEND);

    // Now the string has been rendered to the FBO texture, so all we need to
    // do is create a material and attach it to the quad.
    
    // Create a texture wrapper from the texture handle in the FBO.
    gfxcore::CTexture* pTexture =
        mp_Assets->Create<gfxcore::CTexture>(this->GetOwner());
    pTexture->LoadFromExisting(FBO.GetTexture());

    // Retrieve the raw data.
    const unsigned char* data =
        reinterpret_cast<const unsigned char*>(pTexture->GetData());

    // Load the texture wrapper with raw data, because the FBO will go out
    // of scope soon, thus destroying the texture handle, but the data needs
    // to be preserved.
    pTexture->LoadFromRaw(GL_RGBA8, GL_RGBA, totals.x, totals.y, data);

    gfx::CMaterial M(*mp_Assets);
    if(!M.LoadEffect(gfx::EffectType::GRAYSCALE) ||
       !M.LoadTexture(*pTexture))
    {
        delete[] data;
        delete[] verts;
        delete[] inds;

        FBO.Destroy() && VAO.Destroy();
        return false;
    }

    gfx::CQuad Q(*mp_Assets, totals.x, totals.y);
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

void CFont::ClearString()
{
    m_str.str(std::string());
}

bool CFont::Destroy()
{
    m_size = 18;
    for(auto i : m_glyphData)
        mp_Assets->Delete(i.second.texture);
    m_glyphData.clear();
    this->ClearString();
    return true;
}

bool CFont::LoadGlyph(const char c, const uint16_t index)
{
    ZEN_ASSERTM(mp_Assets != nullptr, "an asset manager must be attached");
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
                << "Empty character bitmap for '" << c << '\'' << CLog::endl;
    }

    // Create the OpenGL texture and store the FreeType bitmap
    // in it as a monochrome texture.

    // Store for restoring later.
    GLint pack;
    GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &pack));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    gfxcore::CTexture* pTexture = mp_Assets->Create<gfxcore::CTexture>(this->GetOwner());
    pTexture->LoadFromRaw(GL_R8, GL_RED, w, h, bitmap.buffer);
    std::stringstream ss;
    ss << "Size " << m_size << " texture bitmap for '" << c << '\'';
    pTexture->SetFilename(ss.str());

    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, pack));

    // Store the glyph internally.
    glyph_t glyph;
    glyph.texture   = pTexture;
    glyph.size      = math::vector_t(w, h);
    glyph.position  = math::Vector<int32_t>(slot->metrics.horiBearingY >> 6,
                                            slot->metrics.horiBearingX >> 6);
    glyph.advance   = slot->advance.x >> 6;
    m_glyphData[c]  = glyph;
    
    // Clean up TTF data.
    FT_Done_Glyph(g);
    
    return true;
}

void CFont::AttachManager(asset::CAssetManager& Assets)
{
    mp_Assets = &Assets;
}

void CFont::SetColor(const color4f_t& Color)
{
    m_Color = Color;
}

uint16_t CFont::GetTextWidth(const string_t& text) const
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

uint16_t CFont::GetTextHeight(const string_t& text) const
{
    if(text.empty()) return 0;

    uint16_t lines = 1;
    
    auto i = text.begin(),
         j = text.end();
         
    for( ; i != j; ++i)
        if((*i) == '\n') ++lines;
        
    return lines * m_height;
}
