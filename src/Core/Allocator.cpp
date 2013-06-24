#include "Zenderer/Core/Allocator.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

CAllocator::CAllocator(size_t initial /*= (1 << 16)*/) :
    CSubsystem("Allocator"),
    mp_head(nullptr),
    mp_ptr(nullptr),
    m_size(0)
{
    m_blocks.clear();
    m_size = this->Pad(initial);
}

bool CAllocator::Init()
{
    if(m_init) return true;

    mp_head = new char[m_size];
    mp_ptr  = mp_head;

    memset(mp_head, '\0', m_size);

    m_blocks.push_back(m_size - 1);

    return (m_init = true);
}

bool CAllocator::Destroy()
{
    if(!m_init) return false;

    // Check for memory leaks (not that it matters).
    size_t total = 0;
    for(auto& i : m_blocks)
    {
        if(!IsFree(i))
        {
            m_Log   << m_Log.SetSystem("Allocator")
                    << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << i << " byte memory leak found."
                    << CLog::endl;
        }

        total += i;
    }

    delete[] mp_head;
    return !(m_init = false);
}

void* CAllocator::operator()(size_t bytes)
{
    this->Pad(bytes);
    void* ptr = nullptr;

    size_t& end = *(m_blocks.rbegin());

    // We have available space at the end
    if(IsFree(end) && end >= bytes)
    {
        if(end - bytes > 0)
            m_blocks.push_back(end - bytes);

        end = bytes;
        ptr = mp_ptr;

        // Adjust to the next free block
        mp_ptr += bytes;
    }

    // Nothing, so we need to start a search from the beginning.
    else
    {
        auto i = m_blocks.begin(), j = m_blocks.end();
        size_t total = 0;

        for( ; i != j; ++i)
        {
            size_t& n = *i;

            // Free block found
            if(IsFree(n) && n >= bytes)
            {
                // Insert a free block specifier
                if(n - bytes > 0)
                    m_blocks.insert(i, n - bytes);

                // Modify the current block
                n = bytes;

                // Return a pointer to the current block
                ptr = mp_head + total;
            }

            total += n;
        }
    }

    if(ptr == nullptr)
    {
        m_Log   << m_Log.SetSystem("Allocator")
                << m_Log.SetMode(LogMode::ZEN_FATAL)
                << "Out of memory." << CLog::endl;
    }
    else
    {
#ifdef _DEBUG
        static uint32_t alloc = 0;

        alloc += bytes;
        m_Log   << m_Log.SetSystem("Allocator")
                << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << "Allocated " << bytes << " bytes [0x" << ptr
                << "] (" << alloc << " total)." << CLog::endl;
#endif // _DEBUG
    }

    return ptr;
}

size_t CAllocator::operator[](void* ptr)
{
    return this->Free(ptr);
}

size_t CAllocator::Free(void* ptr)
{
    auto i = m_blocks.begin(),
         j = m_blocks.end();

    size_t index = 0;
    size_t offset = 0;

    for( ; i != j; ++i, ++index)
    {
        size_t& bytes = *i;

        if(!IsFree(bytes) && mp_head + offset == ptr)
        {
            size_t freed = bytes;

            // Mark as free.
            bytes -= 1;

            // If the previous or next block is also free, we
            // should combine them to limit fragmentation.
            auto it = m_blocks.begin();
            auto cp = i;

            for(size_t n = 1; n < index; ++n) ++it;
            ++cp;

            size_t& left    = *it;
            size_t& right   = *cp;

            if(IsFree(left) && it != i)
            {
                left += bytes + 1;
                i = m_blocks.erase(i);
            }

            if(IsFree(right))
            {
                right += bytes + 1;
                m_blocks.erase(i);
            }

#ifdef _DEBUG
            static uint32_t total_freed = 0;
            total_freed += freed + 1;

            m_Log   << m_Log.SetSystem("Allocator")
                    << m_Log.SetMode(LogMode::ZEN_DEBUG)
                    << "Freed " << freed << " bytes ("
                    << total_freed << " total)." << CLog::endl;
#endif // _DEBUG
            // Freed an even number of bytes
            return freed;
        }

        // + 1 to offset for odd blocks.
        offset += (IsFree(bytes)) ? bytes + 1 : bytes;
    }

    return 0;
}

CAllocator& CAllocator::Get()
{
    static CAllocator Alloc;
    Alloc.Init();
    return Alloc;
}
