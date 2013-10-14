/**
 * @file
 *  Zenderer/Levels/LevelLoader.hpp - A loader for @a Zenderer level files.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
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
 * @addtogroup Levels
 * @{
 **/

#ifndef ZENDERER__LEVELS__LEVEL_LOADER_HPP
#define ZENDERER__LEVELS__LEVEL_LOADER_HPP

#include <regex>

#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Graphics/Polygon.hpp"
#include "Zenderer/Graphics/Scene.hpp"
#include "Zenderer/Utilities/Utilities.hpp"
#include "Zenderer/Utilities/FileParser.hpp"
#include "Level.hpp"

namespace zen
{
namespace lvl
{
    /// Loads levels (go figure).
    class ZEN_API zLevelLoader
    {
    public:
        zLevelLoader(gfx::zScene& Scene, asset::zAssetManager& Assets) :
            m_Log(util::zLog::GetEngineLog()),
            m_Scene(Scene), m_Assets(Assets)
        {
            m_levels.clear();
        }

        ~zLevelLoader()
        {
            for(auto& i : m_levels)
            {
                /// @todo
                for(auto& j : i.lights)
                    ;//m_Scene.RemoveLight(j);

                for(auto& j : i.entities)
                    m_Scene.RemoveEntity(*j);

                i.valid = false;
            }

            m_levels.clear();
        }

        /// @todo Error checking.
        virtual bool LoadFromFile(const string_t& filename)
        {
            using std::stoi;
            using std::stod;

            std::ifstream file(filename);
            if(!file)
            {
                util::zLog& Log = util::zLog::GetEngineLog();
                Log << Log.SetMode(util::LogMode::ZEN_ERROR)
                    << Log.SetSystem("Level")
                    << "Failed to open level: '" << filename << "'."
                    << util::zLog::endl;
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

            // Back up so we see "<entity>" again for parsing in the loop.
            //file.seekg(-10, std::ios::cur);

            // Find each block of data and parse it.
            std::vector<string_t> parts;
            while(std::getline(file, line))
            {
                util::stripl(line);
                if(line.empty() || (line.size() >= 2 && line.substr(0, 2) == "//"))
                    continue;

                else if(line.find("<entity") == 0)
                {
                    gfx::zPolygon Poly(m_Assets);
                    obj::zEntity& Latest = m_Scene.AddEntity();
                    Parser.LoadFromStreamUntil(file, "</entity>", file.tellg(),
                                               filename.c_str(), true);

                    parts = util::split(Parser.PopResult("position", "0,0"), ',');

                    Latest.Move(stod(parts[0]), stod(parts[1]),
                                stoi(Parser.PopResult("depth", "1")));

                    string_t result = Parser.PopResult("vertex");
                    while(!result.empty())
                    {
                        parts = util::split(result, ',');
                        Poly.AddVertex(math::vector_t(stod(parts[0]), stod(parts[1]), 1.0));
                        result = Parser.PopResult("vertex");
                    }

                    if(Parser.Exists("color"))
                    {
                        std::vector<string_t> parts = util::split(Parser.PopResult("color"), ',');
                        Poly.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));
                    }

                    parts = util::split(Parser.PopResult("indices"), ',');
                    if(!parts.empty() && !parts[0].empty())
                    {
                        std::vector<gfxcore::index_t> indices;
                        indices.reserve(parts.size());
                        for(auto& i : parts) indices.emplace_back(stoi(std::move(i)));
                        Poly.SetIndices(indices);
                    }

                    result = Parser.PopResult("stretch");
                    if(util::zFileParser::ResultToBool(result))
                    {
                        /// @todo
                    }

                    /// @todo Polygons vs. Entities differentiation.
                    if(Parser.Exists("texture"))
                    {
                        Latest.LoadFromTexture(Parser.PopResult("texture"));
                    }
                    else
                    {
                        Latest.AddPrimitive(std::move(Poly.Create()));
                    }

                    result = Parser.PopResult("attributes", "0x00");
                    uint8_t attr = this->ParseAttribute(result);
                    /// @todo

                    level.entities.emplace_back(&Latest);

                    enum class AttributeType : uint8_t
                    {
                        NONE     = 0x00,
                        PHYSICAL = 0x01,
                        INVISIBLE= 0x02
                    };

                    if(attr & static_cast<uint8_t>(AttributeType::PHYSICAL))
                    {
                        level.physical.emplace_back(&Latest);
                    }

                    if(attr & static_cast<uint8_t>(AttributeType::INVISIBLE))
                    {
                        Latest.Disable();
                    }
                }

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
                    point.position = math::vector_t(stod(parts[0]), stod(parts[1]));

                    point.whitelist = util::split(Parser.PopResult("whitelist"), ',');
                    point.blacklist = util::split(Parser.PopResult("blacklist"), ',');
                    level.spawnpoints.emplace_back(std::move(point));
                }

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
                    Light.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));
                    Light.SetBrightness(stod(Parser.PopResult("brightness")));

                    if(Light.GetType() != gfx::LightType::ZEN_AMBIENT)
                    {
                        parts = util::split(Parser.PopResult("position"), ',');
                        Light.SetPosition(stod(parts[0]), stod(parts[1]));

                        parts = util::split(Parser.PopResult("attenuation"), ',');
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
                }

                else
                {
                    m_Log << m_Log.SetMode(util::LogMode::ZEN_DEBUG)
                          << m_Log.SetSystem("Level") << "Unrecognized line: "
                          << line << util::zLog::endl;
                }
            }

            level.valid = (!level.entities.empty() || !level.lights.empty());
            if(level.valid) m_levels.emplace_back(std::move(level));
            return level.valid;
        }

        bool PopLevel(level_t& lvl)
        {
            if(m_levels.empty()) return false;
            lvl = m_levels.front();
            m_levels.erase(m_levels.begin());
            return true;
        }

    private:
        uint8_t ParseAttribute(const string_t& hex)
        {
#ifndef __GNUC__
            return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
#else
            return static_cast<uint8_t>(std::strtoul(hex.c_str(), nullptr, 16));
#endif // __GNUC__
        }

        util::zLog& m_Log;
        gfx::zScene& m_Scene;
        asset::zAssetManager& m_Assets;
        std::vector<level_t> m_levels;
    };
}
}

#endif // ZENDERER__LEVELS__LEVEL_LOADER_HPP

/** @} **/
