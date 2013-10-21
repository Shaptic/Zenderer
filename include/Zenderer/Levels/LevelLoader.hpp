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

#ifndef __GNUC__
#  include <regex>
#endif // __GNUC__

#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Graphics/ConcavePolygon.hpp"
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
        enum class AttributeType : uint8_t
        {
            NONE     = 0x00,
            PHYSICAL = 0x01,
            INVISIBLE= 0x02,
            ANIMATION= 0x04
        };

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
                for(auto& j : i.lights)
                    m_Scene.RemoveLight(*j);

                for(auto& j : i.entities)
                    m_Scene.RemoveEntity(*j.second);

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
                    gfx::zConcavePolygon Poly(m_Assets);
                    obj::zEntity& Latest = m_Scene.AddEntity();
                    Parser.LoadFromStreamUntil(file, "</entity>", file.tellg(),
                                               filename.c_str(), true);

                    parts = util::split(Parser.PopResult("position", "0,0"), ',');

                    Latest.Move(stod(parts[0]), stod(parts[1]),
                                stoi(Parser.PopResult("depth", "1")));

                    string_t result = Parser.PopResult("vertex");
                    uint16_t poly = 0;
                    while(!result.empty())
                    {
                        parts = util::split(result, ',');
                        Poly.AddVertex(math::vector_t(stod(parts[0]), stod(parts[1]), 1.0));
                        result = Parser.PopResult("vertex");
                        poly++;
                    }

                    if(Parser.Exists("color"))
                    {
                        std::vector<string_t> parts = util::split(Parser.PopResult("color"), ',');
                        Poly.SetColor(stod(parts[0]), stod(parts[1]), stod(parts[2]));
                    }

                    result = Parser.PopResult("stretch");
                    if(util::zFileParser::ResultToBool(result))
                    {
                        /// @todo
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
                        gfx::zQuad Quad(m_Assets, Poly.CalcW(), Poly.CalcH());
                        Quad.SetRepeating(true);
                        if(poly == 4) pActive = &Quad;

                        // Load / Attach a texture to the polygon if we can.
                        if(Parser.Exists("texture"))
                        {
                            gfx::zMaterial M(m_Assets);
                            M.LoadTextureFromFile(Parser.PopResult("texture"));
                            pActive->AttachMaterial(M);
                        }

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
                        level.physical.emplace_back(&Latest);
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
                    point.position = math::vector_t(stod(parts[0]), stod(parts[1]));

                    point.whitelist = util::split(Parser.PopResult("whitelist"), ',');
                    point.blacklist = util::split(Parser.PopResult("blacklist"), ',');
                    level.spawnpoints.emplace_back(std::move(point));
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
                    level.lights.emplace_back(&Light);
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

        static obj::zEntity* GetEntityByID(const level_t& level,
#ifdef ZEN_DEBUG_BUILD
                                           const string_t& id)
#else
                                           const uint32_t id)
#endif // ZEN_DEBUG_BUILD
        {
            auto it = level.entities.find(id);
            return (it == level.entities.end()) ? nullptr : it->second;
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

/**
 * @class zen::lvl::zLevelLoader
 * @details
 *  @a Zenderer provides a level format specification, combined with a level
 *  editor, in order to speed up development and facilitate uniform level
 *  processing. The specification is outlined below as a sample file.
 *
 * @code
 *  // Sample Zenderer level file.
 *  //  This file contains a simple side-scroller level with
 *  //  a single raised platform, a tiled floor, a light,
 *  //  a player, and two enemies / spawn points.
 *
 *  // SECTION: Metadata [optional]
 *  //  This is typically ignored, but could be used for providing
 *  //  the player with details while the level is being loaded.
 *
 *  author=George Kudrayvtsev
 *  description=A sample level provided for testing purposes
 *
 *  // SECTION: Entities
 *  //  This section contains mesh data for creating a variety of
 *  //  textured primitives. See the entity-mesh file format for
 *  //  more details.
 *
 *  // The game background that cannot be interacted with.
 *  // It's just a simple quad placed at (0, 0, 1) in the game world.
 *  // We also add an identifier so that the loader can directly
 *  // reference this object. When exporting release builds, this should
 *  // be pre-processed as a hash value to speed up lookups.
 *  <entity>
 *      depth=0
 *      position=0,0
 *      texture=sky.png
 *      id=background
 *  </entity>
 *
 *  // The ground. This uses a special parameter to indicate that the
 *  // entity should tile the texture to fit the vertices, rather than
 *  // stretching. Here we also see the emergence of the 'attributes' value
 *  // in the entity specification. This is a flag carrier that has a variety
 *  // of options that can be enabled in it. In this case, 0x01 indicates
 *  // that the entity reacts physically with the world, with collision
 *  // detection and such.
 *  <entity>
 *      depth=1
 *      position=0,0
 *
 *      vertex=0,0
 *      vertex=0,800
 *      vertex=800,600
 *      vertex=600,0
 *
 *      stretch=false
 *      texture=rock.png
 *      attributes=0x01
 *  </entity>
 *
 *  // 2 platforms on which the enemies will stand on.
 *  <entity>
 *      depth=2
 *      texture=platform.png
 *      attributes=0x01
 *  </entity>
 *
 *  // SECTION: Spawn Points
 *  //  This section contains detailed spawn point info, specifying who's
 *  //  allowed to spawn where.
 *  //
 *  //  The 'whitelist' attribute can be used to specify exactly who is
 *  //  allowed to spawn at a certain point, whereas 'blacklist' can be
 *  //  used to allowed all but those on the list to spawn at a point.
 *  //  Since we try to maintain ambiguity about the functionality of the
 *  //  game, white/blacklists can only be defined based on a texture name.
 *  //
 *  //  We define the spawn type with an XML-like attribute
 *  //  syntax. We prefer to use all caps for the values to prevent
 *  //  dealing with mixed-case string comparisons when loading.
 *
 *  // Players are defined by a spawn point (or multiple,
 *  // in which case one is chosen at random), and an entity mesh. Since
 *  // we defined the entity mesh above, we must provide the spawn point
 *  // now, as well.
 *  <spawn type="PLAYER">
 *      position=10,400
 *  </spawn>
 *
 *  <spawn type="ENEMY">
 *      position=100, 300
 *      whitelist=enemy1.png,enemy2.png
 *  </spawn>
 *
 *  <spawn type="ENEMY">
 *      position=100,700
 *      whitelist=enemy1.png
 *  </spawn>
 *
 *  // SECTION: Lighting
 *  //  Lighting is critical to the ambience and realism of a level. We define
 *  //  several types of lights and create them very similarly to the spawn
 *  //  point specification.
 *
 *  // Ambient light to illuminate the whole scene slightly.
 *  <light type="AMBIENT">
 *      color=1.00,1.00,1.00
 *      brightness=0.50
 *  </light>
 *
 *  // A "torch," but without a texture.
 *  <light type="POINT">
 *      position=100,100
 *      color=1.00,1.00,0.00
 *      brightness=0.15
 *      attenuation=0.05,0.01,0.00
 *  </light>
 * @endcode
 *
 * @todo    Split into two files.
 **/

/** @} **/
