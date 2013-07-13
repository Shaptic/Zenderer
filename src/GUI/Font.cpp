#include "Zenderer/GUI/Font.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gui::CFont;

CFont::CFont(const void* const owner) :
    CAsset(owner), m_size(18) {}

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
                << "FreeType2 API is not initialized."
                << CLog::endl;
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

const void* const CFont::GetData() const
{
    return reinterpret_cast<const void* const>(&mp_glyphData);
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
        const math::rect_t& Dim = mp_glyphData[letter].dim;

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
        w += Dim.w;
        h  = math::max<uint16_t>(h, Dim.h + h);
    }

    // Render all of the loaded data onto a texture,
    // then assign that texture to the entity.
    gfxcore::DrawBatch D;
    gfxcore::CVertexArray VAO(GL_STATIC_DRAW);
    gfx::CRenderTarget FBO(w, h);

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

    gfxcore::CRenderer::BlendOperation(gfxcore::BlendFunc::STANDARD_BLEND);

    for(size_t i = 0, j = text.length(); i < j; ++i)
    {
        // Render each character (skip if unrenderable).
        if(text[i] > '~' || text[i] < ' ') continue;

        mp_glyphData[text[i]].texture->Bind();
        GL(glDrawElements(GL_TRIANGLES, 6, gfxcore::INDEX_TYPE,
            (void*)(sizeof(gfxcore::index_t) * i * 6)));
    }

    gfxcore::CRenderer::ResetMaterialState();
    gfxcore::CRenderer::BlendOperation(gfxcore::BlendFunc::DISABLE_BLEND);

    // Now the string has been rendered to the FBO texture,
    // so all we need to do is create a material and attach
    // it to the quad.
    gfx::CMaterial* M = new gfx::CMaterial(*mp_Assets);
    if(!M->LoadEffect(gfx::EffectType::GRAYSCALE) ||
       !M->LoadTextureFromHandle(FBO.GetTexture()))
    {
        delete[] verts;
        delete[] inds;
        delete M;

        return false;
    }

    gfx::CQuad Q(w, h);
    Q.AttachMaterial(M);
    Q.Create();

    Ent.AddPrimitive(Q);
    Ent.Create();

    delete[] verts;
    delete[] inds;
    delete M;

    return FBO.Destroy() && VAO.Destroy();
}

void CFont::ClearString()
{
    m_str.str(std::string());
}

bool CFont::Destroy()
{
    return FT_Done_Face(m_FontFace) == 0;
    m_size = 18;
    for(auto i : mp_glyphData)
        mp_Assets->Delete(i.second.texture);
    mp_glyphData.clear();
    this->ClearString();
}


bool CFont::LoadGlyph(const char c, const uint32_t index)
{
    ZEN_ASSERTM(mp_Assets != nullptr, "an asset manager must be attached");
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
    uint32_t h = bitmap.rows;

    // Create the OpenGL texture and store the FreeType bitmap
    // in it as a monochrome texture.

    // Store for restoring later.
    GLint pack;
    GL(glGetIntegerv(GL_UNPACK_ALIGNMENT, &pack));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1))

    gfxcore::CTexture* pTexture = mp_Assets->Create<gfxcore::CTexture>(this->GetOwner());
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

    mp_glyphData[c] = glyph;
    return true;
}
