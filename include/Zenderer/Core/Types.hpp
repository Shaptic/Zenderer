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
 * @addtogroup Core
 *  Contains core elements used throughout the entire engine, like vertex
 *  types and error handling operations.
 * @{
 **/

#ifndef ZENDERER__CORE__TYPES_HPP
#define ZENDERER__CORE__TYPES_HPP

#include <cstdint>
#include <string>

/// The engine can be built as a DLL, then linked with the generated .lib.
#ifdef ZENDERER_EXPORTS
  #define ZEN_API __declspec(dllexport)
#else
  #ifdef _DEBUG
    #define ZEN_API
  #else
    #define ZEN_API __declspec(dllimport)
  #endif // _DEBUG
#endif // ZENDERER_EXPORTS

/// An all-encompassing namespace for any and all components of @a Zenderer.
namespace zen
{
    /**
     * Specifies whether the engine should use 'double' (64-bit) or 'real_t'
     * (32-bit) values for real_t numbers.
     **/
    typedef
#ifdef ZEN_DOUBLE_PRECISION
    double
#else
    float
#endif // ZEN_DOUBLE_PRECISION
    real_t;

    /**
     * Character sequence representation used throughout the engine.
     *  Currently, this just defaults to `std::string`, but there may be
     *  a future implementation in which there is a custom string class
     *  created tailored specifically for the engine.
     **/
    typedef std::string string_t;

    /**
     * Clamps a value in the provided range [min, max].
     *
     * @param   val     Value to clamp
     * @param   minimum Minimum
     * @param   maximum Maximum
     **/
    template<typename T>
    ZEN_API void clamp(T& val, const T& minumum, const T& maximum)
    {
        if(val < minumum)       val = minumum;
        else if(val > maximum)  val = maximum;
    }

    /**
     * A 32-bit floating point color component (RGBA).
     *  Typically, this is expected to be in the range [0, 1].
     **/
    struct ZEN_API color4f_t
    {
        color4f_t(const real_t r = 0.f,
                  const real_t g = 0.f,
                  const real_t b = 0.f,
                  const real_t a = 1.f) : r(r), g(g), b(b), a(a) {}

        void Clamp()
        {
            clamp<real_t>(r, 0, 1);
            clamp<real_t>(g, 0, 1);
            clamp<real_t>(b, 0, 1);
            clamp<real_t>(a, 0, 1);
        }

        real_t r,   ///< Red
               g,   ///< Green
               b,   ///< Blue
               a;   ///< Alpha
    };

    /**
     * A 24-bit floating point color component (RGB).
     *  Typically, this is expected to be in the range [0, 1].
     **/
    struct ZEN_API color3f_t
    {
        color3f_t(const real_t r = 0.0,
                  const real_t g = 0.0,
                  const real_t b = 0.0) : r(r), g(g), b(b) {}

        void Clamp()
        {
            clamp<real_t>(r, 0, 1);
            clamp<real_t>(g, 0, 1);
            clamp<real_t>(b, 0, 1);
        }

        real_t r,   ///< Red
               g,   ///< Green
               b;   ///< Blue
    };
}

#endif // ZENDERER__CORE__TYPES_HPP

/** @} **/

