/**
 * @file
 *  Zenderer/Core/Globals.hpp - A singleton class controlling execution order of global variable initialization.
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
 * @{
 **/

#ifndef ZENDERER__CORE__GLOBALS_HPP
#define ZENDERER__CORE__GLOBALS_HPP

#include "Zenderer/Utilities/Log.hpp"
#include "Zenderer/Core/Allocator.hpp"
#include "Zenderer/Utilities/Settings.hpp"

namespace zen
{
    class ZEN_API CGlobals
    {
    public:
        ~CGlobals(){}

        bool Init()
        {
            if(!s_init)
            {
                s_init = (s_Log.Init() && s_Alloc.Init() &&
                          s_Settings.Init());
            }

            return s_init;
        }

        static CGlobals& GetGlobals()
        {
            static CGlobals Globals;
            if(!Globals.Init())
            {
                std::cerr << "[FATAL] Globals -- Failed to initialize "
                    << "global systems.\n";
                exit(1);
            }

            return Globals;
        }

        static zen::util::CLog& GetLog()
        {
            return CGlobals::s_Log;
        }

        static zen::CAllocator& GetAllocator()
        {
            return CGlobals::s_Alloc;
        }

        static zen::util::CSettings& GetSettings()
        {
            return CGlobals::s_Settings;
        }

    private:
        CGlobals(){}

        static zen::util::CLog      s_Log;
        static zen::CAllocator      s_Alloc;
        static zen::util::CSettings s_Settings;
        static bool                 s_init;
    };

    /// Globally accessible module to access globals.
    static CGlobals& g_Globals = CGlobals::GetGlobals();

    /// Globally accessible log file used by the engine.
    static zen::util::CLog& g_EngineLog = CGlobals::GetLog();

    /// Globally accessible memory allocator used by the engine.
    static zen::CAllocator& g_Alloc = CGlobals::GetAllocator();

    /// Globally accessible settings data used by the engine.
    static zen::util::CSettings& g_Settings = CGlobals::GetSettings();

    bool CGlobals::s_init = false;

}   // namespace zen

#endif // ZENDERER__CORE__GLOBALS_HPP

/** @} **/
