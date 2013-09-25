#include "Zenderer/Objects/Entity.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using obj::zEntity;

zEntity::zEntity(asset::zAssetManager& Assets) :
    m_Assets(Assets), m_Log(util::zLog::GetEngineLog()),
    m_MV(math::matrix4x4_t::GetIdentityMatrix()),
    m_sort(0), m_depth(1), m_inv(false), m_enabled(true)
{
}

zEntity::~zEntity()
{
    this->Destroy();
}

bool zEntity::LoadFromFile(const string_t& filename)
{
    ZEN_ASSERT(!filename.empty());

    util::zParser Parser;
    std::ifstream file(filename);
    std::string line;
    uint32_t line_no = 0;

    if(!file) return false;

    gfx::zPolygon*  pPrim = nullptr;
    gfx::zMaterial* pMat  = nullptr;

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
                mp_allPrims.emplace_back(pPrim);
                delete pMat;
                pMat = nullptr;
            }

            pPrim = new gfx::zQuad(m_Assets, 0, 0);
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
            pMat = new gfx::zMaterial(m_Assets);
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
                static_cast<gfx::zQuad*>(pPrim)->Resize(
                    Parser.GetValuei("width"), Parser.GetValuei("height"));

            else
                static_cast<gfx::zQuad*>(pPrim)->Resize(
                    pMat->GetTexture().GetWidth(),
                    pMat->GetTexture().GetHeight());

            if(Parser.Exists("invert"))
                static_cast<gfx::zQuad*>(pPrim)->SetInverted(
                    Parser.GetValueb("invert"));

            if(Parser.Exists("repeat"))
                static_cast<gfx::zQuad*>(pPrim)->SetRepeating(
                    Parser.GetValueb("repeat"));

            pPrim->AttachMaterial(*pMat);
        }
    }

    m_filename = filename;
    return true;
}

bool zEntity::LoadFromTexture(const string_t& filename)
{
    this->Destroy();

    gfx::zMaterial Mat(m_Assets);
    if(!Mat.LoadTextureFromFile(filename)) return false;

    gfx::zQuad* pPrimitive = new gfx::zQuad(m_Assets,
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

bool zEntity::AddPrimitive(const gfx::zPolygon& Polygon)
{
    if(!(Polygon.m_DrawData.icount && Polygon.m_DrawData.vcount)) return false;

    gfx::zPolygon* pPoly = new gfx::zPolygon(Polygon);
    pPoly->AttachMaterial(const_cast<gfx::zMaterial&>(Polygon.GetMaterial()));
    //pPoly->SetInverted(m_inv);
    pPoly->SetColor(Polygon.m_Color);
    pPoly->Create();
    mp_allPrims.push_back(pPoly);

    m_PolyBB = math::rect_t(pPoly->GetLeftPoint(), pPoly->GetLowPoint(),
                            pPoly->GetW(), pPoly->GetH());

    m_Box = math::aabb_t(math::rect_t(this->GetX() + m_PolyBB.x,
                                      this->GetY() + m_PolyBB.y,
                            math::max<uint32_t>(this->GetW(), m_PolyBB.w),
                            math::max<uint32_t>(this->GetH(), m_PolyBB.h)));

    // Reset then set the material flag.
    //m_sort &= 0xFFFFFFFF ^ gfxcore::zSorter::MATERIAL_FLAG;
    //m_sort |= (pQuad->GetMaterial().GetID() << gfxcore::zSorter::MATERIAL_OFFSET);
    return true;
}

bool zEntity::Optimize()
{
    ZEN_ASSERTM(false, "not implemented");
    return false;
}

bool zEntity::Draw(bool is_bound /*= false*/)
{
    if(!m_enabled) return false;

    for(auto& i : mp_allPrims)
        i->Draw(is_bound);

    return true;
}

void zEntity::Move(const math::vector_t& Pos)
{
    this->Move(Pos.x, Pos.y, Pos.z);
}

void zEntity::Move(const real_t x, const real_t y, const real_t z /*= 1.0*/)
{
    math::vector_t d = math::vector_t(x, y, z) - this->GetPosition();

    for(auto& i : mp_allPrims)      i->Move(x, y, z);
    for(auto& i : m_Triangulation)  i = i + d;

    m_MV.Translate(math::vector_t(x, y, z));
    m_Box = math::aabb_t(math::rect_t(x + m_PolyBB.x, y + m_PolyBB.y,
                                      this->GetW(), this->GetH()));
}

void zEntity::Adjust(const real_t dx, const real_t dy, const real_t dz /*= 0.0*/)
{
    this->Move(this->GetPosition() + math::vector_t(dx, dy, dz));
}

void zEntity::Adjust(const math::vector_t& delta)
{
    this->Move(this->GetPosition() + delta);
}

void zEntity::Offload(gfxcore::zVertexArray& VAO, const bool keep /*= true*/)
{
    for(auto& i : mp_allPrims)
        i->LoadIntoVAO(VAO, keep);
}

bool zEntity::Offloaded() const
{
    for(auto& i : mp_allPrims)
    {
        if(!(i->m_DrawData.Vertices && i->m_DrawData.Indices))
            return false;
    }

    return true;
}

bool zEntity::Collides(const zEntity& Other, math::vector_t* poi)
{
    if(!m_Box.collides(Other.m_Box)) return false;
    for(auto& i : mp_allPrims)
    {
        for(auto& j : mp_allPrims)
            if(i.Collides(j, poi)) return true;
    }
    
    return false;
}

bool zEntity::Collides(const math::rect_t& other)
{
    return this->Collides(math::aabb_t(other), poi);
}

bool zEntity::Collides(const math::aabb_t& other)
{
    if(!m_Box.collides(other)) return false;
    for(auto& i : mp_allPrims)
    {
        if(i->Collides(other)) return true;
    }
    
    return false;
}

bool zEntity::Collides(const math::vector_t& pos)
{
    return this->Collides(math::rect_t(pos.x, pos.y, 1, 1), poi);
}

void zEntity::SetDepth(uint8_t depth)
{
    // Limit depth to 8-bit values (256).
    clamp<uint8_t>(depth, 0U, 1U << 8);
    m_depth = depth;
    //m_sort &= (0xFFFFFFFF ^ gfxcore::zSorter::DEPTH_FLAG);
    //m_sort |= (depth << gfxcore::zSorter::DEPTH_OFFSET);
}

const math::matrix4x4_t& zEntity::GetTransformation() const
{
    return m_MV;
}

math::vector_t zEntity::GetPosition() const
{
    return math::vector_t(m_MV[0][3], m_MV[1][3], m_MV[2][3]);
}

const math::aabb_t& zEntity::GetBox() const
{
    return m_Box;
}

uint32_t zEntity::GetSortFlag() const
{
    return m_sort;
}

std::vector<gfx::zPolygon*>::const_iterator zEntity::begin() const
{
    return mp_allPrims.cbegin();
}

std::vector<gfx::zPolygon*>::const_iterator zEntity::end() const
{
    return mp_allPrims.cend();
}

void zEntity::Destroy()
{
    for(auto& i : mp_allPrims)
    {
        delete i;
        i = nullptr;
    }

    mp_allPrims.clear();
    mp_allPrims.shrink_to_fit();
}

bool zEntity::FileError(const string_t& filename,
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

    m_Log << ")." << zLog::endl;
    return false;
}
