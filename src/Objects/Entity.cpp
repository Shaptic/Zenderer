#include "Zenderer/Objects/Entity.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using obj::CEntity;

CEntity::CEntity(asset::CAssetManager& Assets) :
    m_Assets(Assets), m_Log(util::CLog::GetEngineLog()),
    m_MV(math::matrix4x4_t::GetIdentityMatrix()),
    m_sort(0), m_depth(1), m_inv(false), m_enabled(true)
{
}

CEntity::~CEntity()
{
    this->Destroy();
}

bool CEntity::LoadFromFile(const string_t& filename)
{
    ZEN_ASSERT(!filename.empty());

    util::CINIParser Parser;
    std::ifstream file(filename);
    std::string line;
    uint32_t line_no = 0;

    if(!file) return false;

    gfx::CQuad*     pPrim = nullptr;
    gfx::CMaterial* pMat  = nullptr;

    this->Destroy();

    while(std::getline(file, line))
    {
        ++line_no;
        if(line.empty() || line[0] == '/') continue;

        std::smatch res;
        std::regex float3("=(\\d+\\.?\\d*),(\\d+\\.?\\d*),?(\\d+\\.?\\d*)?\\s*$");

        if(line.find("position=") != std::string::npos &&
           std::regex_match(line, res, float3))
        {
            this->Move(std::stod(res[0]), std::stod(res[1]));
        }

        else if(line.find("primcount") != std::string::npos &&
                std::regex_match(line, res, std::regex("=(\\d+)\\s*$")))
        {
            mp_allPrims.reserve(std::stoi(res[0]));
        }

        else if(line.find("<prim>") != std::string::npos)
        {
            // We've already loaded a primitive before.
            if(pPrim != nullptr)
            {
                mp_allPrims.push_back(pPrim);
                delete pMat;
                pMat = nullptr;
            }

            pPrim = new gfx::CQuad(m_Assets, 0, 0);
            const std::streampos start = file.tellg();

            // Find end of primitive block.
            while(std::getline(file, line) &&
                  line.find("</prim>") == std::string::npos);

            const std::streampos end = file.tellg();

            Parser.LoadFromStream(file, start, end, filename.c_str());

            // We have loaded key=value pairs for a primitive instance.
            // Now it's time to specify things based on material.

            // The material can be loaded from the filestream since we have
            // a texture=, vshader=, and fshader= (or just the texture) which
            // is a valid .zfx file format.
            pMat = new gfx::CMaterial(m_Assets);
            if(Parser.Exists("material") && !pMat->LoadFromFile(Parser.GetValue("material")))
            {
                delete pPrim; delete pMat;
                this->Destroy();
                return this->FileError(filename, line, line_no, ErrorType::BAD_MATERIAL);
            }

            else if(!pMat->LoadFromStream(file, start, end))
            {
                delete pPrim; delete pMat;
                this->Destroy();
                return this->FileError(filename, line, line_no, ErrorType::NO_TEXTURE);
            }

            if(Parser.Exists("width") && Parser.Exists("height"))
                pPrim->Resize(Parser.GetValuei("width"),
                              Parser.GetValuei("height"));

            else
                pPrim->Resize(pMat->GetTexture().GetWidth(),
                              pMat->GetTexture().GetHeight());

            if(Parser.Exists("invert"))
                pPrim->SetInverted(Parser.GetValueb("invert"));

            if(Parser.Exists("repeat"))
                pPrim->SetRepeating(Parser.GetValueb("repeat"));

            pPrim->AttachMaterial(*pMat);
        }
    }

    m_filename = filename;
    return true;
}

bool CEntity::LoadFromTexture(const string_t& filename)
{
    this->Destroy();

    gfx::CMaterial Mat(m_Assets);
    if(!Mat.LoadTextureFromFile(filename)) return false;

    gfx::CQuad* pPrimitive = new gfx::CQuad(m_Assets,
        Mat.GetTexture().GetWidth(),
        Mat.GetTexture().GetHeight());

    pPrimitive->AttachMaterial(Mat);
    pPrimitive->SetInverted(m_inv);
    pPrimitive->Create();
    pPrimitive->SetColor(color4f_t(1, 1, 1, 1));

    mp_allPrims.push_back(pPrimitive);
    m_Box = math::aabb_t(math::rect_t(this->GetX(), this->GetY(),
                                      pPrimitive->GetW(),
                                      pPrimitive->GetH()));
    return true;
}

bool CEntity::AddPrimitive(const gfx::CQuad& Quad)
{
    if(Quad.m_DrawData.icount == 0 || Quad.m_DrawData.vcount == 0) return false;

    gfx::CQuad* pQuad = new gfx::CQuad(Quad);
    pQuad->AttachMaterial(const_cast<gfx::CMaterial&>(Quad.GetMaterial()));
    pQuad->SetInverted(m_inv);
    pQuad->Create();
    pQuad->SetColor(Quad.m_DrawData.Vertices[0].color);
    mp_allPrims.push_back(pQuad);

    m_Box = math::aabb_t(math::rect_t(this->GetX(), this->GetY(),
                            math::max<uint32_t>(this->GetW(), pQuad->GetW()),
                            math::max<uint32_t>(this->GetH(), pQuad->GetH())));

    // Reset then set the material flag.
    //m_sort &= 0xFFFFFFFF ^ gfxcore::CSorter::MATERIAL_FLAG;
    //m_sort |= (pQuad->GetMaterial().GetID() << gfxcore::CSorter::MATERIAL_OFFSET);
    return true;
}

bool CEntity::Optimize()
{
    ZEN_ASSERTM(false, "not implemented");
    return false;
}

bool CEntity::Draw(bool is_bound /*= false*/)
{
    if(!m_enabled) return false;

    for(auto& i : mp_allPrims)
        i->Draw(is_bound);

    return true;
}

void CEntity::Move(const math::vector_t& Pos)
{
    this->Move(Pos.x, Pos.y, Pos.z);
}

void CEntity::Move(const real_t x, const real_t y, const real_t z /*= 1.0*/)
{
    for(auto& i : mp_allPrims)
        i->Move(x, y, z);

    m_MV.Translate(math::vector_t(x, y, z));
    m_Box = math::aabb_t(math::rect_t(x, y, this->GetW(), this->GetH()));
}

void CEntity::Adjust(const real_t dx, const real_t dy, const real_t dz /*= 0.0*/)
{
    this->Move(this->GetPosition() + math::vector_t(dx, dy, dz));
}

void CEntity::Adjust(const math::vector_t& delta)
{
    this->Move(this->GetPosition() + delta);
}

void CEntity::Offload(gfxcore::CVertexArray& VAO, const bool keep /*= true*/)
{
    for(auto& i : mp_allPrims)
        i->LoadIntoVAO(VAO, keep);
}

bool CEntity::Offloaded() const
{
    for(auto& i : mp_allPrims)
    {
        if(i->m_DrawData.Vertices != nullptr ||
           i->m_DrawData.Indices  != nullptr)
            return false;
    }

    return true;
}

bool CEntity::Collides(const CEntity& Other)
{
    return m_Box.collides(Other.GetBox());
}

bool CEntity::Collides(const math::rect_t& other)
{
    return m_Box.collides(math::aabb_t(other));
}

bool CEntity::Collides(const math::vector_t& pos)
{
    return m_Box.collides(math::aabb_t(pos, math::vector_t(1, 1)));
}

void CEntity::SetDepth(uint8_t depth)
{
    // Limit depth to 8-bit values (256).
    clamp<uint8_t>(depth, 0U, 1U << 8);
    m_depth = depth;
    //m_sort &= (0xFFFFFFFF ^ gfxcore::CSorter::DEPTH_FLAG);
    //m_sort |= (depth << gfxcore::CSorter::DEPTH_OFFSET);
}

const math::matrix4x4_t& CEntity::GetTransformation() const
{
    return m_MV;
}

math::vector_t CEntity::GetPosition() const
{
    return math::vector_t(m_MV[0][3], m_MV[1][3], m_MV[2][3]);
}

const math::aabb_t& CEntity::GetBox() const
{
    return m_Box;
}

uint32_t CEntity::GetSortFlag() const
{
    return m_sort;
}

std::vector<gfx::CQuad*>::const_iterator CEntity::cbegin() const
{
    return mp_allPrims.cbegin();
}

std::vector<gfx::CQuad*>::const_iterator CEntity::cend() const
{
    return mp_allPrims.cend();
}

void CEntity::Destroy()
{
    for(auto& i : mp_allPrims)
    {
        delete i;
        i = nullptr;
    }

    mp_allPrims.clear();
    mp_allPrims.shrink_to_fit();
}

bool CEntity::FileError(const string_t& filename,
                        const string_t& line, const uint32_t line_no,
                        const ErrorType& Err)
{
    m_Log << m_Log.SetMode(LogMode::ZEN_ERROR) << m_Log.SetSystem("Entity")
          << "Error while parsing '" << filename << "' on line " << line_no
          << ": " << line << "(";

    switch(Err)
    {
    case ErrorType::BAD_PAIR:
        m_Log << "bad key=value pair";
        break;

    case ErrorType::BAD_POSITION:
        m_Log << "position must at least contain x,y coordinates";
        break;

    case ErrorType::BAD_MATERIAL:
        m_Log << "failed to load material file";
        break;

    case ErrorType::NO_TEXTURE:
        m_Log << "no texture specified for primitive";
        break;

    default:
        m_Log << "unknown parsing error";
        break;
    }

    m_Log << ")." << CLog::endl;
    return false;
}
