/**
 * @file
 *  Zenderer/Core/Types.hpp - Defines standard types used extensively
 *  throughout almost every part of the engine.
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
 *  Contains core elements used throughout the entire engine, like vertex
 *  types and error handling operations.
 * @{
 **/

#ifndef ZENDERER__CORE__TYPES_HPP
#define ZENDERER__CORE__TYPES_HPP

#include <cstdint>
#include <string>

#define _HAS_EXCEPTIONS 0

#ifdef __GNUC__

// For whatever reason, even with -std=c++11,
// or -std=c++0x, or -std=gnu++11 enabled,
// these functions aren't found.

#include <sstream>
#include <cstdlib>

namespace std
{
    template<typename T>
    static inline std::string to_string(const T& t)
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    static inline int stoi(const std::string& s)
    {
        return std::atoi(s.c_str());
    }

    static inline double stod(const std::string& s)
    {
        return std::atof(s.c_str());
    }
}
#endif // __GNUC__

// Attempt to detect debug or release build.
#ifndef ZEN_DEBUG_BUILD
#  if defined(_DEBUG) || defined(DEBUG)
#    pragma message("Compiling in debug mode...")
#    define ZEN_DEBUG_BUILD
#  else
#    pragma message("Compiling in release mode...")
#  endif // defined(_DEBUG)
#endif // ZEN_DEBUG_BUILD

/// Conditionally build as a DLL.
#ifndef ZEN_DLL
#  if defined(ZENDERER_EXPORTS) && defined(_WIN32)
#    define ZEN_DLL 1
#  else
#    define ZEN_DLL 0
#  endif // ZENDERER_EXPORTS
#endif // ZEN_DLL

/// The engine can be built as a DLL, then linked with the generated .lib.
#if ZEN_DLL
#  define ZEN_API __declspec(dllexport)
#else
#  if defined(ZEN_DEBUG_BUILD) && !ZEN_DLL
#    define ZEN_API
#  elif ZEN_DLL
#    define ZEN_API __declspec(dllimport)
#  else
#    define ZEN_API
#  endif // ZEN_DEBUG_BUILD
#endif // ZENDERER_EXPORTS

/// An all-encompassing namespace for any and all components of @a Zenderer.
namespace zen
{
    using real_t = float;

    /**
     * Character sequence representation used throughout the engine.
     *  Currently, this just defaults to `std::string`, but there may be
     *  a future implementation in which there is a custom string class
     *  created tailored specifically for the engine.
     **/
    using string_t = std::string;

    /**
     * @fn zen::clamp
     * Clamps a value in the provided range [min, max].
     *
     * @param   val         Value to clamp
     * @param   minimum     Minimum (inclusive)
     * @param   maximum     Maximum (inclusive)
     **/
    template<typename T>
    ZEN_API void clamp(T& val, const T& minimum, const T& maximum)
    {
        if(val < minimum)       val = minimum;
        else if(val > maximum)  val = maximum;
    }

    /**
     * A 32-bit floating point color component (RGBA).
     *  Typically, this is expected to be in the range [0, 1].
     **/
    struct ZEN_API color4f_t
    {
        real_t  r,  ///< Red
                g,  ///< Green
                b,  ///< Blue
                a;  ///< Alpha

        color4f_t(const real_t r = 1.f,
                  const real_t g = 1.f,
                  const real_t b = 1.f,
                  const real_t a = 1.f) : r(r), g(g), b(b), a(a) {}
    };

    /**
     * A 24-bit floating point color component (RGB).
     *  Typically, this is expected to be in the range [0, 1].
     **/
    struct ZEN_API color3f_t
    {
        real_t  r,  ///< Red
                g,  ///< Green
                b;  ///< Blue

        color3f_t(const real_t r = 1.0,
                  const real_t g = 1.0,
                  const real_t b = 1.0) : r(r), g(g), b(b) {}
    };
}

#endif // ZENDERER__CORE__TYPES_HPP

/** @} **/
