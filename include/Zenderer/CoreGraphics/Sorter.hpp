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

#ifndef ZENDERER__CORE_GRAPHICS__SORTER_CPP
#define ZENDERER__CORE_GRAPHICS__SORTER_CPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Objects/Entity.hpp"

namespace zen
{
namespace gfxcore
{
    /**
     * A collection of sort methods to optimize rendering.
     *  This is a static class with no member functions, and contains
     *  tools to sort an entity by material, depth, and alpha value.
     *  It utilizes a high-speed, low-level bit comparison technique
     *  to make comparisons as fast as possible.
     *
     *  You can read more about the sorting architecture in the documentation
     *  for the ic::gfx::CSceneManager class.
     **/
    class ZEN_API CSorter
    {
    public:        
        static inline const CEntity*
        SortByMaterial(const CEntity* pEnt1, const CEntity* pEnt2)
        {
            return CSorter::SortBy(pEnt1, pEnt2, MATERIAL_FLAG);
        }
        
        static inline const CEntity*
        SortByDepth(const CEntity* pEnt1, const CEntity* pEnt2)
        {
            return CSorter::SortBy(pEnt1, pEnt2, DEPTH_FLAG);
        }
        
        static inline const CEntity*
        SortByAlpha(const CEntity* pEnt1, const CEntity* pEnt2)
        {
            return CSorter::SortBy(pEnt1, pEnt2, ALPHA_FLAG);
        }
        
        static inline uint32_t
        CreateSortFlag(const uint32_t material_id,  const uint32_t depth_value,
                       const uint32_t alpha_bit,    const uint32_t unused = 0)
        {
            return  (material_id << MATERIAL_OFFSET)| 
                    (depth_value << DEPTH_OFFSET)   | 
                    (alpha_bit   << ALPHA_OFFSET)   | 
                    (unused      << UNUSED_OFFSET);
        }
        
        static const uint32_t MATERIAL_FLAG     = 0xFFF00000;
        static const uint32_t MATERIAL_OFFSET   = 20; // 20 bits into the flag

        static const uint32_t DEPTH_FLAG        = 0x000FF000;
        static const uint32_t DEPTH_OFFSET      = 12;

        static const uint32_t ALPHA_FLAG        = 0x00000800;
        static const uint32_t ALPHA_OFFSET      = 11;

        static const uint32_t UNUSED_FLAG       = 0x000007FF;
        static const uint32_t UNUSED_OFFSET     = 0;
        
    private:
        // Not implemented; purely static class.
        CSorter(); ~CSorter();
        
        static inline const CEntity*
        SortBy(const CEntity* pEnt1, const CEntity* pEnt2, const uint32_t flag)
        {
            return (pEnt1->GetSortFlag() & flag) < (pEnt2->GetSortFlag() & flag)
                ?   pEnt1 : pEnt2;
        }
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__SORTER_CPP

/** @} **/
