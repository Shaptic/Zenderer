/**
 * @file
 *  Zenderer/Core/Allocator.hpp - A high-speed allocator using a static memory stack.
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
 * @addtogroup Engine
 * @{
 **/

#ifndef ZENDERER__CORE__ALLOCATOR_HPP
#define ZENDERER__CORE__ALLOCATOR_HPP

#include <list>

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Core/Subsystem.hpp"

namespace zen
{
    /// An optional custom allocator for high-speed memory requests.
    class ZEN_API zAllocator : public zSubsystem
    {
    public:
        ~zAllocator();

        bool Init();
        bool Destroy();

        /// Allocates `bytes` of memory.
        void* operator()(size_t bytes = 4);

        /// Frees `ptr` from memory.
        size_t operator[](void* ptr);

        size_t Free(void* ptr);

        template<typename T> inline
        T* get(size_t count = 1)
        {
            return (T*)this->operator()(count * sizeof(T));
        }

        /// Singleton accessor.
        static zAllocator& Get();

        /// Default allocation size, in bytes.
        static const uint16_t ALLOC_SIZE = 4;

    private:
        static inline
        size_t Pad(size_t& bytes)
        {
            size_t i = bytes % ALLOC_SIZE;

            return (!i) ? bytes : (bytes += ALLOC_SIZE - i);
        }

        static inline
        bool IsFree(const size_t block)
        {
            return ((block & 0x1) == 1);
        }

        /// Pre-allocated block of memory.
        zAllocator(const size_t initial = 1 << 16);

        std::list<size_t> m_blocks;

        char*   mp_head;
        char*   mp_ptr;
        size_t  m_size;
    };
}   // namespace zen

#endif // ZENDERER__CORE__ALLOCATOR_HPP

// Detailed doxygen docs.

/**
 * @class zen::zAllocator
 * @details
 * Let us assume a memory structure like so:
 *
 * <pre>
 * 0000 0000 0000 0000  0000 0000 0000 0000  0000 0000 0000 0000
 * </pre>
 *
 * Thus we store an internal `std::list<int>` containing only a single
 * value: `{ 47 }`.
 * 47 because there are 48 free bytes, and we differentiate between free
 * and used bytes by making the number odd if they are free.
 *
 * When we want to allocate 9 bytes (for example, a non-padded
 * `struct` with a `char` and a `double` member), the memory structure
 * would change to look like this:
 *
 * <pre>
 * _______________  Next block pointer
 *                |
 *                V
 * 1111 1111 1111 0000  0000 0000 0000 0000  0000 0000 0000 0000
 * ^
 * |______ Head pointer
 * </pre>
 *
 * We pad the 9 bytes to 12 for alignment purposes (assuming a
 * `zAllocator::BLOCK_SIZE` of 4), and now our interal byte `list`
 * contains `{ 12, 35 }` because we have an occupied block of 12
 * bytes (even) followed by a free block of 35 bytes (odd).
 *
 * Let's assume again that we allocate 32 bytes, and then free our first
 * 12 byte block. Now the memory structure looks like this:
 *
 * <pre>
 *            Next block pointer ___________________________
 *                                                          |
 *                                                          V
 * 0000 0000 0000 1111  1111 1111 1111 1111  1111 1111 1111 0000
 * ^
 * |______ Head pointer
 * </pre>
 *
 * and our internal byte `list` contains `{ 11, 32, 3 }` for 12 free
 * bytes, 32 used bytes, and 4 free bytes, respectively.
 *
 * Finally, let's assume the user requests an allocation of 8 bytes.
 * From the next block pointer (shown above), there is no viable 8-byte
 * chunk to allocate, so the search begins from the head.
 *
 * This is actually done by analyzing the internal memory `list`. With
 * a request of 8 bytes and a final free block of 3 bytes (since the
 * next block pointer always points to the last free block, so `list.end()`)
 * we start the search from `list.begin()`, giving us 11 which we know as
 * a free block of 12 bytes. Thus we can return the head pointer and make
 * the list contain `{ 8, 3, 32, 3}` and a memory structure like this:
 *
 * <pre>
 *            ___ Next block pointer
 *           |
 *           V
 * 1111 1111 0000 1111  1111 1111 1111 1111  1111 1111 1111 0000
 * ^
 * |______ Head pointer
 * </pre>
 *
 * As you can see, the memory becomes fairly fragmented, and a subsequent
 * request to allocate 8 bytes would fail, despite there being 8 bytes
 * available. This is impossible to remedy without a lot of slow memory
 * movement, which would in turn cause all of the allocated pointers to be
 * completely invalid on the user end.
 *
 * @example Allocation
 *
 * @section Dynamic Memory Allocation
 * @subsection Warning
 *  This example is deprecated and `zAllocator` is not used in @a Zenderer.
 *
 * @code
 *  // Allocate char[4]
 *  char* data0 = g_Alloc.get<char>(4);
 *  char* data1 = g_Alloc(sizeof(char) * 4);
 *
 *  // Allocate int
 *  int* data2 = g_Alloc();             // assuming sizeof(int) == 4
 *  int* data3 = g_Alloc(sizeof(int));
 *  int* data4 = g_Alloc.get<int>(1);
 *
 * // Manually freeing the memory
 *  g_Alloc.Free(data0);
 *
 *  // Double-deletion is safe, but will log an error
 *  g_Alloc.Free(data0);
 *
 *  // Have to call destructor manually.
 *  DynamicLight->~zLight();
 *  g_Alloc.Free(DynamicLight);
 * @endcode
 *
 **/

/** @} **/
