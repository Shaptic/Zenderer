/**
 * @file
 *  Zenderer/Utilities/Assert.hpp - A debug utility for verifying
 *  program state at runtime.
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

#ifndef ZENDERER__UTILITIES__ASSERT_HPP
#define ZENDERER__UTILITIES__ASSERT_HPP

// String manipulation
#include <sstream>
#include <string>

// Screen output
#include <iostream>

#include "Zenderer/Core/Types.hpp"
#include "Log.hpp"

// Assertions only run in debug builds.
#ifdef _DEBUG
  /// Assertion macro for convenience
  #define ZEN_ASSERT(expression) \
    zen::util::runtime_assert(expression, #expression, __LINE__, __FILE__)

  /// Assertion with a custom message
  #define ZEN_ASSERTM(expression, msg)    \
    zen::util::runtime_assert(expression, \
        #expression, __LINE__, __FILE__, msg)

#else
  #define ZEN_ASSERT(expr)
  #define ZEN_ASSERTM(expr, msg)
#endif // _DEBUG

namespace zen
{
namespace util
{
    /**
     * Executes a verification of an expression, crashing on error.
     *
     * @param   expr        Boolean expression to assert on
     * @param   expr_str    The boolean expression as a string (for output)
     * @param   line_no     The line number the assertion is occuring on
     * @param   file_name   The file the assertion occurs in
     * @param   msg         A custom output message (optional=`nullptr`)
     **/
    ZEN_API void runtime_assert(const bool      expr,
                                string_t        expr_str,
                                const size_t    line_no,
                                string_t        file_name,
                                const char*     msg = nullptr);
}   // namespace util
}   // namespace ic

#endif // ZENDERER__UTILITIES__ASSERT_HPP

/**
 * @fn zen::util::runtime_assert()
 * @details
 *  This function plays nicely with zen::util::CLog::GetEngineLog(),
 *  using it to store the fatal error messages to output them directly
 *  to the screen.
 *  This function can, but likely should not be called directly,
 *  since it relates to the preprocessor macro `ZEN_ASSERT` (defined
 *  above) to evaluate the given expression as a string.
 *  The stored error is as follows:
 *
 *       [FATAL] Assert -- `expression_str` failed at line `line_no` of `file_name`: `_msg_`
 *
 *  The variable names above are replaced with their respective values,
 *  naturally.
 *
 *  If this is used with the `ZEN_ASSERT` macro, this will ONLY
 *  actually do things in Debug builds, with `_DEBUG` specified.
 **/

/** @} **/
