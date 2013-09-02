/**
 * @file
 *  Zenderer/Utilities/Random.hpp - A thin abstraction layer over the STL
 *  `<random>` utilities.
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
 * @addtogroup Utilities
 * @{
 **/

#ifndef ZENDERER__UTILITIES__RANDOM_HPP
#define ZENDERER__UTILITIES__RANDOM_HPP

#include <cstdlib>
#include <random>

#include "Zenderer/Core/Types.hpp"
#include "Assert.hpp"

namespace zen
{
namespace util
{
    /// Facilitates simplification of random number generation.
    template<int SEED = 0, typename RNG = std::mt19937>
    class ZEN_API CRandom
    {
    public:
        CRandom()
        {
            m_RNG.seed(SEED == 0 ? time(nullptr) : SEED);
        }

        /**
         * Generates a random integer from the range [`low`, `hi`).
         *
         * @tparam  T       An integer type (`long`, `uinnt16_t`, etc.)
         * @param   low     The lower bound
         * @param   hi      The upper bound
         *
         * @return  An integer of type `T` in the range [low, hi).
         **/
        template<typename T>
        T randint(const T& low, const T& hi)
        {
            std::uniform_int_distribution<T> d(low, hi);
            return d(m_RNG);
        }

        /**
         * Generates a random real number from the range [`low`, `hi`).
         *
         * @tparam  T       A floating-point type (`double`, `real_t`, etc.)
         * @param   low     The lower bound
         * @param   hi      The upper bound
         *
         * @return  A real number of type `T` in the range [low, hi).
         **/
        template<typename T>
        T randreal(const T& low, const T& hi)
        {
            std::uniform_real_distribution<T> d(low, hi);
            return d(m_RNG);
        }

        /**
         * Chooses a random element in the given iterator range.
         *
         * @tparam  InputIterator   An iterable type, typically deduced.
         * @param   begin           The starting iterator
         * @param   end             The ending iterator
         *
         * @warning Modifying the return value of this method call **will**
         *          change the contents of the original container.
         *
         * @return  A **reference** to an element in the container.
         **/
        template<typename InputIterator>
        auto choice(InputIterator begin, InputIterator end) -> decltype(*begin)&
        {
            int d = std::distance(begin, end);
            ZEN_ASSERT(d >= 0);
            std::advance(tmp, this->randint(0, d));
            return *tmp;
        }

    private:
        RNG m_RNG;
    };
}
}

#endif // ZENDERER__UTILITIES__RANDOM_HPP

/**
 * @class zen::util::CRandom
 * @details
 *  This object provides a thin abstraction layer over the C++11
 *  `<random>` implementation. It removes the cumbersome instantiation of
 *  unweildy templates. It also provides convenience member functions
 *  that allow for easily generating ranges of numbers, or choosing
 *  a random element of a container.
 *
 *  Here's the difference between creating a random integer in
 *  the range [1, 10) between this wrapper and the standard STL:
 *
 *  @code
 *  // STL
 *  std::mt19937 Mersenne;
 *  std::uniform_int_distribution<int> Range(1, 10);
 *  Mersenne.seed(time(nullptr));
 *  int r = Range(Mersenne);
 *
 *  // Zenderer
 *  util::CRandom<0, std::mt19937> Mersenne;
 *  int r = Mersenne.randint(1, 10);
 *
 *  // Zenderer with defaults
 *  util::CRandom<> Mersenne;   // Uses std::mt19937 by default.
 *  int r = Mersenne.randint(1, 10);
 *  @endcode
 *
 *  As you can see, there are no longer complex template names and
 *  parameters to remember. There is merely the optional seed
 *  as the first parameter to util::CRandom, and the optional
 *  random-number generating engine as the second parameter.
 *
 * @example Random
 *
 * @section gen Generating Random Numbers
 *
 *  Preliminary code:
 *  @code
 *  #include "Zenderer/Utilities/Random.hpp"
 *  using zen::util::CRandom;
 *
 *  CRandom<> Generator;
 *  @endcode
 *
 * @subsection int  Integers
 *  @code
 *  Generator.randint(-5, 5);
 *  @endcode
 *
 * @subsection float    Floating-Point
 *  @code
 *  Generator.randreal(-5, 5);
 *  @endcode
 *
 * @subsection choice   Choosing From a Container
 *  @code
 *  // Populate the container with random items.
 *  std::vector<int> lotsOfStuff;
 *  size_t limit = Generator.randint(500, 1000);
 *  lotsOfStuff.reserve(limit);
 *  for(size_t i = 0; i < limit; ++i)
 *  {
 *      lotsOfStuff.push_back(Generator.randint(0, 512));
 *  }
 *
 *  // The actual choosing:
 *  int element = Generator.choice(lotsOfStuff.begin(),
 *                                 lotsOfStuff.end());
 *
 *  // Change a random element to 69:
 *  int& element = Generator.choice(lotsOfStuff.begin(),
 *                                 lotsOfStuff.end());
 *  element = 69;
 *  @endcode
 **/

/** @} **/
