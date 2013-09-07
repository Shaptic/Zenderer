#include "Zenderer/CoreGraphics/Sorter.hpp"

using zen::gfxcore::zSorter;

const zen::obj::zEntity* zSorter::SortBy(const zen::obj::zEntity* pEnt1,
                                         const zen::obj::zEntity* pEnt2,
                                         const uint32_t flag)
{
    return (pEnt1->GetSortFlag() & flag) < (pEnt2->GetSortFlag() & flag)
        ?   pEnt1 : pEnt2;
}
