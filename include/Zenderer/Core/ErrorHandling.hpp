/**
 * @file
 *  Zenderer/Core/ErrorHandling.hpp - Last-resort error handling.
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

#ifndef ZENDERER__CORE__ERROR_HANDLING_HPP
#define ZENDERER__CORE__ERROR_HANDLING_HPP

#include <cstdlib>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
#else
  #include <iostream>
#endif // _WIN32

namespace zen
{
    /**
     * A fall-back error handler with OS-specific implementation.
     *  On Windows, this will pop up a dialog box with the given message
     *  displayed. On *nix systems, this will output to stderr in the
     *  format
     *      `[title] -- message.\n`
     *  Both will exit the program after output.
     *
     *  This error handler should be used as a last-resort fall-back for
     *  when all others fail. The "prettier" error handlers should be used
     *  in most cases, and this one should be used when there is a failure
     *  to load a proper rendering context, a font fails to load, or some
     *  other critical error that would prevent the error window from
     *  creating or displaying properly.
     *
     * @param   message     Error message to output
     * @param   title       Caption on Windows, prefix otherwise.
     *
     * @see     zen::gfx::error_window
     **/
    void error_fallback(const char* message, const char* title = "Error");

}   // namespace zen

#endif // ZENDERER__CORE__ERROR_HANDLING_HPP

/** @} **/
