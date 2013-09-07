/**
 * @file
 *  Zenderer/CoreGraphics/Sorter.hpp - Sorts entites in a variety of
 *  different ways, such as by material and by alpha component.
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
 * @addtogroup CoreGraphics
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__SORTER_HPP
#define ZENDERER__CORE_GRAPHICS__SORTER_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gfxcore
{
    /// A collection of sort methods to optimize rendering.
    class ZEN_API zSorter
    {
    public:
        static inline const obj::zEntity*
        SortByMaterial(const obj::zEntity* pEnt1, const obj::zEntity* pEnt2)
        {
            return zSorter::SortBy(pEnt1, pEnt2, MATERIAL_FLAG);
        }

        static inline const obj::zEntity*
        SortByDepth(const obj::zEntity* pEnt1, const obj::zEntity* pEnt2)
        {
            return zSorter::SortBy(pEnt1, pEnt2, DEPTH_FLAG);
        }

        static inline const obj::zEntity*
        SortByAlpha(const obj::zEntity* pEnt1, const obj::zEntity* pEnt2)
        {
            return zSorter::SortBy(pEnt1, pEnt2, ALPHA_FLAG);
        }

        static inline uint32_t
        CreateSortFlag(const uint16_t material_id,  const uint16_t depth_value,
                       const uint8_t  alpha_bit,    const uint16_t unused = 0)
        {
            return  (material_id << MATERIAL_OFFSET)|
                    (depth_value << DEPTH_OFFSET)   |
                    (alpha_bit   << ALPHA_OFFSET)   |
                    (unused      << UNUSED_OFFSET);
        }

        static const uint32_t MATERIAL_FLAG     = 0xFFFF0000;
        static const uint32_t MATERIAL_OFFSET   = 16; // 24 bits into the flag

        static const uint32_t DEPTH_FLAG        = 0x0000FF00;
        static const uint32_t DEPTH_OFFSET      = 8;

        static const uint32_t ALPHA_FLAG        = 0x00000080;
        static const uint32_t ALPHA_OFFSET      = 7;

        static const uint32_t UNUSED_FLAG       = 0x0000007F;
        static const uint32_t UNUSED_OFFSET     = 0;

    private:
        // Not implemented; purely static class.
        zSorter(); ~zSorter();

        static const obj::zEntity*
        SortBy(const obj::zEntity* pEnt1,
               const obj::zEntity* pEnt2,
               const uint32_t flag);
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SORTER_HPP

/**
 * @class       zen::gfxcore::zSorter
 * @details
 *  This is a static class with no member functions, and contains
 *  tools to sort an entity by material, depth, and alpha value.
 *  It utilizes a high-speed, low-level bit comparison technique
 *  to make comparisons as fast as possible.
 *
 * @section     sort    Optimized Sorting Algorithm
 * @subsection  expl    Explanation
 *  In order to maximize rendering efficiency, it is important to
 *  make as little state changes as possible to the underlying
 *  rendering API. For this, @a Zenderer implements a high-speed
 *  sorting technique that operates at maximum performance.
 *
 *  Essentially, each entity internally stores a 32-bit integer
 *  consisted of various state flags describing material usage,
 *  depth, transparency, and other data.
 *
 *  This state flag is used for comparing the scene graph prior
 *  to rendering in order to, for example, group all entities with
 *  identical materials together in order to minimize texture swaps.
 *
 *  A material is defined as follows:
 *
 *     material = texture + vshader + fshader
 *
 *  And a thorough explanation of the state bit definitions are ahead.
 *
 *  We expect to have no more than 4096 materials in a single game. This
 *  can, of course, be easily expanded, but 4096 is more than enough for
 *  a standard 2D game. For this, we allocate 12 bits in the flag for
 *  storing a unique material ID (2<sup>12</sup> = 4096).
 *
 *  Thus: 32 - 16 = 16 bits remaining.
 *
 *  For depth information, we allocate 8 bits, giving us 256 different
 *  levels of depth for any entity in the game. Again this is perfectly
 *  acceptable for a 2D game.
 *
 *  Thus: 16 - 8 = 8 bits remaining.
 *
 *  For transparency, we only need a single bit. to compare if a
 *
 *  Thus: 8 - 1 = 7 bits remaining.
 *
 *  The remaining bits are reserved for any future sorting requirements.
 *  7 bits gives 128 different values, surely large enough to accommodate
 *  any future requirements.
 *
 * @subsection  algo    Sorting
 *  The actual sorting process works as follows:
 *
 *  The scene stores a list of entities for rendering. It also contains
 *  an internal dictionary associating the various sorting parameters
 *  (material, depth) with a list of entities that use that parameter.
 *
 *  On addition of an entity from the scene, it's inserted into the
 *  appropriate list based on its internal flags.
 **/

/** @} **/
