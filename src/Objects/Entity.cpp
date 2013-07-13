#include "Zenderer/Objects/Entity.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using obj::CEntity;

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

        if(line.find("position") != std::string::npos)
        {
            std::vector<string_t> pair = util::split(line, '=');
            if(pair.size() != 2) return this->FileError(filename, line, line_no);

            pair = util::split(line, ',');
            if(pair.size() < 2)
                return this->FileError(filename, line, line_no,
                                       ErrorType::BAD_POSITION);

            this->Move(std::stod(pair[0]), std::stod(pair[1]));

            // Depth is optional
            if(pair.size() == 3) m_Position.z = std::stod(pair[2]);
        }

        else if(line.find("primcount") != std::string::npos)
        {
            int count = std::stoi(util::split(line, '=')[1]);
            if(count > 0)
            {
                mp_allPrims.reserve(count);
            }
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
    pPrimitive->Create();
    pPrimitive->SetColor(color4f_t(1, 1, 1, 1));

    mp_allPrims.push_back(pPrimitive);
    return true;
}

/// @todo Check for material `nullptr`
bool CEntity::AddPrimitive(const gfx::CQuad& Quad)
{
    // All entity primitives must have viable textures attached.
    if(&Quad.GetMaterial().GetTexture() == 
       &gfxcore::CRenderer::GetDefaultTexture())
        return false;

    gfx::CQuad* pQuad = new gfx::CQuad(Quad);
    pQuad->AttachMaterial(const_cast<gfx::CMaterial&>(Quad.GetMaterial()));
    pQuad->Create();
    pQuad->SetColor(color4f_t(1, 1, 1, 1));
    mp_allPrims.push_back(pQuad);
    return true;
}

void CEntity::Destroy()
{
    for(auto i : mp_allPrims) { delete i; i = nullptr; }
    mp_allPrims.clear();
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
