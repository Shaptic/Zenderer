/**
 * @file
 *  Zenderer/Objects/Physics.hpp - Integrates Box2D.
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
 * @addtogroup Objects
 * @{
 **/

#ifndef ZENDERER__OBJECTS__PHYSICS_HPP
#define ZENDERER__OBJECTS__PHYSICS_HPP

#include <Box2D/Box2D.h>
#include "Zenderer/Core/Types.hpp"

#ifdef ZEN_DEBUG_BUILD
  #pragma comment(lib, "Box2D_DBG.lib")
#else
  #pragma comment(lib, "Box2D.lib")
#endif

namespace zen
{
namespace obj
{
    class ZEN_API CBody
    {
        b2World& m_World;

    public:
        CBody(b2World& World) :
            m_World(World) {}
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__PHYSICS_HPP
