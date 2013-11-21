#include "Zenderer/Levels/LevelLoader.hpp"

using namespace zen;
using util::LogMode;
using util::zLog;

using namespace lvl;

zLevelLoader::zLevelLoader(gfx::zScene& Scene, asset::zAssetManager& Assets) :
    m_Log(util::zLog::GetEngineLog()), m_Scene(Scene), m_Assets(Assets)
{
    while(!m_levels.empty()) m_levels.pop();
}

zLevelLoader::~zLevelLoader()
{
    while(!m_levels.empty())
    {
        level_t& i = m_levels.front();
        for(auto& j : i.lights)
            m_Scene.RemoveLight(*j);

        for(auto& j : i.entities)
            m_Scene.RemoveEntity(*j.second);

        i.valid = false;
        m_levels.pop();
    }
}

bool zLevelLoader::LoadFromFile(const string_t& filename)
{
    using std::stoi;
    using std::stod;

    std::ifstream file(filename);
    if(!file)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_ERROR) << m_Log.SetSystem("Level")
              << "Failed to open level: '" << filename << "'." << zLog::endl;

        return false;
    }

    util::zFileParser Parser;
    level_t level;
    string_t line;

    level.valid = false;

    // The spec. states that the first object tag within a level must
    // be an entity, so we can safely parse everything prior to finding
    // one and consider that to be the metadata.
    Parser.LoadFromStreamUntil(file, "<entity>", 0, filename.c_str());
    level.metadata.author = Parser.GetFirstResult("author");
    level.metadata.description = Parser.GetFirstResult("description");

    // Find each block of data and parse it.
    std::vector<string_t> parts;
    while(std::getline(file, line))
    {
        util::stripl(line);
        if(line.empty() || (line.size() >= 2 && line.substr(0, 2) == "//"))
            continue;

        else if(line.find("<entity") == 0)
        {
            gfx::zConcavePolygon Poly(m_Assets);
            obj::zEntity& Latest = m_Scene.AddEntity();
            Parser.LoadFromStreamUntil(file, "</entity>", file.tellg(),
                                       filename.c_str(), true);

            parts = util::split(Parser.PopResult("position", "0,0"), ',');
            this->Verify(parts, 2, line, "position");

            Latest.Move(stod(parts[0]), stod(parts[1]),
                        stoi(Parser.PopResult("depth", "1")));

            string_t result = Parser.PopResult("vertex");
            uint16_t poly = 0;
            while(!result.empty())
            {
                parts = util::split(result, ',');
                this->Verify(parts, 2, line, "vertex");
                Poly.AddVertex(math::vector_t(stod(parts[0]), stod(parts[1]), 1.0));
                result = Parser.PopResult("vertex");
                poly++;
            }

            if(Parser.Exists("color"))
            {
                std::vector<string_t> parts = util::split(Parser.PopResult("color"), ',');
                this->Verify(parts, 3, line, "color");
                Poly.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));
            }

            // No vertices, so can be loaded from a texture directly.
            if(poly == 0)
            {
                Latest.LoadFromTexture(Parser.PopResult("texture"));
            }

            // There are vertices so attach the texture (if any)
            // to a polygon instance instead.
            else
            {
                gfx::zPolygon* pActive = &Poly;

                // Special case for quads if there are 4 vertices.
                gfx::zQuad Quad(m_Assets, Poly.CalcWidth(), Poly.CalcHeight());
                if(poly == 4)
                {
                    Quad.SetRepeating(true);
                    if(util::zFileParser::ResultToBool(
                        Parser.PopResult("stretch", "false"))
                    ) { Quad.SetRepeating(false); }

                    pActive = &Quad;
                }

                // Load / Attach a texture to the polygon if we can.
                if(Parser.Exists("texture"))
                {
                    gfx::zMaterial M(m_Assets);
                    M.LoadTextureFromFile(Parser.PopResult("texture"));
                    pActive->AttachMaterial(M);
                }

                // Set inversion toggle.
                pActive->SetInverted(util::zFileParser::ResultToBool(
                    Parser.PopResult("invert", "false")));

                Latest.AddPrimitive(std::move(pActive->Create()));
            }

            // Parse and handle attributes as needed.
            result = Parser.PopResult("attributes", "0x00");
            uint8_t attr = this->ParseAttribute(result);
            level.entities[
#ifndef ZEN_DEBUG_BUILD
                util::string_hash(
#endif // ZEN_DEBUG_BUILD
                Parser.PopResult("id", "entity")
#ifndef ZEN_DEBUG_BUILD
                )
#endif // ZEN_DEBUG_BUILD
            ] = &Latest;

            if(attr & static_cast<uint8_t>(AttributeType::PHYSICAL))
            {
                level.physical.push_back(&Latest);
            }

            if(attr & static_cast<uint8_t>(AttributeType::INVISIBLE))
            {
                Latest.Disable();
            }
        }

        // Handle spawn points.
        else if(line.find("<spawn") == 0)
        {
            // G++ doesn't have regex support for C++11 yet.
#ifdef __GNUC__
            size_t start    = line.find('"');
            size_t end      = line.find('"', start+1);
            const string_t t= line.substr(start+1, end - start - 1);
#else
            std::smatch type;
            if(!std::regex_match(line, type,
                                 std::regex("<spawn type=\"([A-Z]+)\">",
                                    std::regex_constants::icase)))
            {
                /// @todo
                return false;
            }
            const string_t& t = type[1];
#endif // __GNUC__

            spawn_t point;
            point.type = SpawnType::ENEMY_SPAWN;

            if      (t == "PLAYER") point.type = SpawnType::PLAYER_SPAWN;
            else if (t == "ITEM")   point.type = SpawnType::ITEM_SPAWN;

            Parser.LoadFromStreamUntil(file, "</spawn>", file.tellg(),
                                       filename.c_str(), true);

            parts = util::split(Parser.PopResult("position"), ',');
            this->Verify(parts, 2, line, "position");
            point.position = math::vector_t(stod(parts[0]), stod(parts[1]));

            point.whitelist = util::split(Parser.PopResult("whitelist"), ',');
            point.blacklist = util::split(Parser.PopResult("blacklist"), ',');
            level.spawnpoints.push_back(std::move(point));
        }

        // Handle lighting.
        else if(line.find("<light type=\"") == 0)
        {
#ifdef __GNUC__
            size_t start  = line.find('"');
            size_t end    = line.find('"', start+1);
            string_t t    = line.substr(start+1, end - start - 1);
#else
            std::smatch type;
            if(!std::regex_match(line, type,
                                 std::regex("<light type=\"([A-Z]+)\">",
                                    std::regex_constants::icase)))
            {
                /// @todo
                return false;
            }
            const string_t& t = type[1];
#endif // __GNUC__

            gfx::LightType lType = gfx::LightType::ZEN_AMBIENT;
                 if (t == "POINT")  lType = gfx::LightType::ZEN_POINT;
            else if (t == "SPOT")   lType = gfx::LightType::ZEN_SPOTLIGHT;

            gfx::zLight& Light = m_Scene.AddLight(lType);
            Light.Enable();

            Parser.LoadFromStreamUntil(file, "</light>", file.tellg(),
                                       filename.c_str(), true);

            parts = util::split(Parser.PopResult("color", "1,1,1"), ',');
            this->Verify(parts, 3, line, "color");
            Light.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));
            Light.SetBrightness(stod(Parser.PopResult("brightness")));

            if(Light.GetType() != gfx::LightType::ZEN_AMBIENT)
            {
                parts = util::split(Parser.PopResult("position"), ',');
                this->Verify(parts, 2, line, "position");
                Light.SetPosition(stod(parts[0]), stod(parts[1]));

                parts = util::split(Parser.PopResult("attenuation"), ',');
                this->Verify(parts, 3, line, "attenuation");
                Light.SetAttenuation(stod(parts[0]),
                                     stod(parts[1]),
                                     stod(parts[2]));

                if(Light.GetType() == gfx::LightType::ZEN_SPOTLIGHT)
                {
                    Light.SetMaximumAngle(stod(Parser.PopResult("maxangle")));
                    Light.SetMinimumAngle(stod(Parser.PopResult("minangle")));
                }
            }

            Light.Disable();
            level.lights.push_back(&Light);
        }

        else
        {
            m_Log << m_Log.SetMode(LogMode::ZEN_DEBUG)
                  << m_Log.SetSystem("Level") << "Unrecognised line: "
                  << line << zLog::endl;
        }
    }

    level.valid = (!level.entities.empty() || !level.lights.empty());
    if(level.valid) m_levels.emplace(std::move(level));
    return level.valid;
}

bool zLevelLoader::PopLevel(level_t& lvl)
{
    if(m_levels.empty()) return false;
    lvl = std::move(m_levels.front());
    m_levels.pop();
    return true;
}

obj::zEntity* zLevelLoader::GetEntityByID(const level_t& level, const
#ifdef ZEN_DEBUG_BUILD
                                          string_t& id)
#else
                                          uint32_t id)
#endif // ZEN_DEBUG_BUILD
{
    auto it = level.entities.find(id);
    return (it == level.entities.end()) ? nullptr : it->second;
}

uint8_t zLevelLoader::ParseAttribute(const string_t& hex)
{
#ifndef __GNUC__
    return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
#else
    return static_cast<uint8_t>(std::strtoul(hex.c_str(), nullptr, 16));
#endif // __GNUC__
}

bool zLevelLoader::Verify(const std::vector<string_t>& parts,
                          const uint16_t expected_size,
                          const string_t& line,
                          const char* const error_str)
{
    if(parts.size() != expected_size)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_ERROR)
              << m_Log.SetSystem("Level")
              << "Invalid specifier, expected " << expected_size << " "
              << error_str << " values. (line: '" << line << "')" << zLog::endl;

#ifdef ZEN_DEBUG_BUILD
        ZEN_ASSERTM(parts.size() == expected_size, line.c_str());
#endif // ZEN_DEBUG_BUILD
        return false;
    }

    return true;
}
