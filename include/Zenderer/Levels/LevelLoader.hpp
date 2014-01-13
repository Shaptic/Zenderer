/**
 * @file
 *  Zenderer/Levels/LevelLoader.hpp - A loader for @a Zenderer level files.
 *
 * @author      George (@_Shaptic)
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

#include <queue>

#include "Zenderer/Assets/AssetManager.hpp"
#include "Zenderer/Graphics/ConcavePolygon.hpp"
#include "Zenderer/Graphics/Scene.hpp"
#include "Zenderer/Utilities/Utilities.hpp"
#include "Zenderer/Utilities/FileParser.hpp"
#include "Level.hpp"

namespace zen
{

/// An API for easily creating and loading levels.
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
        zLevelLoader(gfx::zScene& Scene, asset::zAssetManager& Assets);
        virtual ~zLevelLoader();

        virtual bool LoadFromFile(const string_t& filename);
        bool PopLevel(level_t& lvl);

        static obj::zEntity* GetEntityByID(const level_t& level, const
#ifdef ZEN_DEBUG_BUILD
                                           string_t& id
#else
                                           uint32_t id
#endif // ZEN_DEBUG_BUILD
        );

    private:
        uint8_t ParseAttribute(const string_t& hex);
        bool Verify(const std::vector<string_t>& parts,
                    const uint16_t expected_size,
                    const string_t& line,
                    const char* const error_str);

        util::zLog& m_Log;
        gfx::zScene& m_Scene;
        asset::zAssetManager& m_Assets;
        std::queue<level_t> m_levels;
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
