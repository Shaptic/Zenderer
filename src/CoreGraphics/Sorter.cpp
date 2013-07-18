#include "Zenderer/CoreGraphics/Sorter.hpp"

using zen::gfxcore::CSorter;

const zen::obj::CEntity* CSorter::SortBy(const zen::obj::CEntity* pEnt1,
                                         const zen::obj::CEntity* pEnt2,
                                         const uint32_t flag)
{
    return (pEnt1->GetSortFlag() & flag) < (pEnt2->GetSortFlag() & flag)
        ?   pEnt1 : pEnt2;
}

