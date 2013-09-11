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
            m_Scene(Scene), m_Assets(Assets), m_Log(util::zLog::GetEngineLog())
        {
            m_levels.clear();
        }

        ~zLevelLoader()
        {
            for(auto& i : m_levels)
            {
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
            std::ifstream file(filename);
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
            file.seekg(-10, std::ios::cur);

            // Find each block of data and parse it.
            std::vector<string_t> parts;
            while(std::getline(file, line))
            {
                util::strip(line);
                if(line.empty() || (line.size() >= 2 &&
                                    line[0] == '/' &&
                                    line[1] == '/'))
                    continue;

                else if(line.find("<entity>") == 0)
                {
                    gfx::zPolygon Poly(m_Assets);
                    obj::zEntity& Latest = m_Scene.AddEntity();
                    Parser.LoadFromStreamUntil(file, "</entity>", file.tellg(),
                                               filename.c_str());

                    parts = util::split(Parser.PopResult("position", "0,0"), ',');

                    Latest.SetDepth(std::stoi(Parser.PopResult("depth", "1")));
                    Latest.Move(std::stod(parts[0]), std::stod(parts[1]));

                    string_t result;
                    do
                    {
                        result = Parser.PopResult("vertex");
                        parts = util::split(result, ',');
                        Poly.AddVertex(math::vector_t(std::stod(parts[0]), std::stod(parts[1])));
                    } while(!result.empty());
                    
                    parts = util::split(Parser.PopResult("indices"), ',');
                    if(!parts.empty())
                    {
                        std::vector<gfxcore::index_t> indices(parts.size());
                        Poly.SetIndices(indices);
                    }

                    result = Parser.PopResult("stretch");
                    if(util::zFileParser::ResultToBool(result))
                    {
                        /// @todo
                    }

                    gfx::zMaterial M(m_Assets);
                    result = Parser.PopResult("texture");
                    M.LoadTextureFromFile(result);
                    Poly.AttachMaterial(M);

                    result = Parser.PopResult("attributes", "0x00");
                    uint8_t attr = this->ParseAttribute(result);
                    /// @todo

                    level.entities.push_back(&Latest);
                }

                /// @todo Differentiate between player and enemy spawns.
                else if(line.find("<spawn") == 0)
                {
                    spawn_t point;
                    Parser.LoadFromStreamUntil(file, "</spawn>",
                                               file.tellg(), filename.c_str());
                    parts = util::split(Parser.PopResult("position"), ',');
                    point.position = math::vector_t(std::stod(parts[0]), std::stod(parts[1]));

                    point.whitelist = util::split(Parser.PopResult("whitelist"), ',');
                    point.blacklist = util::split(Parser.PopResult("blacklist"), ',');
                }

                else if(line.find("<light type=\"") == 0)
                {
                    using std::stod;

                    gfx::zLight& Light = m_Scene.AddLight(gfx::LightType::ZEN_AMBIENT);

                    size_t start = strlen("<light type=\"");
                    size_t end = line.length() - start - strlen(">");
                    string_t type = line.substr(start, end);
                    if(type == "POINT")
                    {
                        Light.SetType(gfx::LightType::ZEN_POINT);
                    }
                    else if(type == "SPOT")
                    {
                        Light.SetType(gfx::LightType::ZEN_SPOTLIGHT);
                    }

                    Parser.LoadFromStreamUntil(file, "</light>", file.tellg(),
                                               filename.c_str());

                    parts = util::split(Parser.PopResult("color"), ',');
                    Light.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));

                    Light.SetBrightness(stod(
                        Parser.PopResult("brightness", "0.01")));

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
                }

                else
                {
                    m_Log << m_Log.SetMode(util::LogMode::ZEN_DEBUG)
                          << m_Log.SetSystem("Level") << "Unrecognized line: "
                          << line << util::zLog::endl;
                }
            }

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
            return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
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
