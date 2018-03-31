/**
 * @file
 *    DLLMain.cpp - Contains the DLL entry point for the @a Zenderer engine.
 *
 * @author      george (halcyon)
 * @version     1.1
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").\n
 *  You may not use this file except in compliance with the License.\n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0 \n
 *  Unless required by applicable law or agreed to in writing, software\n
 *  distributed under the License is distributed on an "AS IS" BASIS,\n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and\n
 *  limitations under the License.
 *
 * @addtogroup Engine
 * @{
 **/

#ifdef ZENDERER_EXPORTS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Zenderer/Zenderer.hpp"
#ifdef ZEN_DEBUG_BUILD
  #undef  ZEN_DEBUG_BUILD
#endif

/**
 * The entry point for the Zenderer engine DLL.
 *
 * @param   HMODULE Module
 * @param   DWORD   Reason for calling function
 * @param   void*   Reserved data that has no use in this case
 *
 * @return  `true`, always.
 **/
bool APIENTRY DllMain(HMODULE hModule, DWORD reason, void* preserved)
{
    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return true;
}

#endif // ZENDERER_EXPORTS

/** @} **/
