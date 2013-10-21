/**
 * @file
 *  Zenderer/Levels/Level.hpp - Defines basic level-related data structures
 *  that contain various info about a game world.
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
 *  This group encompasses a high-level API suitable for easily loading,
 *  importing, and creating highly customizable levels in order to facilitate
 *  rapid development of game worlds.
 *
 * @{
 **/

#ifndef ZENDERER__LEVELS__LEVEL_HPP
#define ZENDERER__LEVELS__LEVEL_HPP

#include <vector>

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Objects/Entity.hpp"
#include "Zenderer/Graphics/Light.hpp"

namespace zen
{
namespace lvl
{
    /// Various spawn type identifiers.
    enum class SpawnType
    {
        PLAYER_SPAWN,
        ENEMY_SPAWN,
        ITEM_SPAWN
    };

    /// Contains spawn point information.
    struct spawn_t
    {
        SpawnType               type;
        math::vector_t          position;
        std::vector<string_t>   whitelist;
        std::vector<string_t>   blacklist;
    };

    /// Contains level metadata.
    struct metalevel_t
    {
        string_t author;
        string_t description;
    };

    /// Contains all level data.
    struct level_t
    {
        metalevel_t                 metadata;
        std::vector<spawn_t>        spawnpoints;
        std::map<
#ifdef ZEN_DEBUG_BUILD
            string_t,
#else
            uint32_t,
#endif // ZEN_DEBUG_BUILD
            obj::zEntity*
        >                           entities;
        std::vector<obj::zEntity*>  physical;
        std::vector<gfx::zLight*>   lights;
        bool                        valid;
    };
}
}

#endif // ZENDERER__LEVELS__LEVEL_HPP

/**
 * @struct zen::lvl::level_t
 * @details
 *  This structure contains all data loaded from level files.
 *  You can directly access pointers to all objects in the level,
 *  as well as physical objects (for collision purposes), spawn
 *  points (for dynamic object placement), lights, and metadata.
 **/

/** @} **/
