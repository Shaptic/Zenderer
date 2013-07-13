/**
 * @file
 *  Zenderer/Core/Subsystem.hpp - An abstract base class for all 'systems'
 *  in the engine, like the asset manager, file handler, etc.
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

#ifndef ZENDERER__CORE__SUBSYSTEM_HPP
#define ZENDERER__CORE__SUBSYSTEM_HPP

#include <vector>

#include "Types.hpp"
#include "Zenderer/Utilities/Log.hpp"

namespace zen
{
    /// An abstract base class for all engine subsystems.
    class ZEN_API CSubsystem
    {
    public:
        /// Sets subsystem to an uninitialized state.
        CSubsystem(const string_t name = "Subsystem");

        /// Destructor does absolutely nothing.
        virtual ~CSubsystem();

        /**
         * Initializes the subsystem.
         *  Any parameters should be passed through the derived class's
         *  constructor.
         **/
        virtual bool Init() = 0;

        /**
         * Cleans up the subsystem completely.
         *  Typically, a subsystem can be re-initialized by setting new
         *  parameters via various Set* methods and calling Init() again,
         *  but this is specific to the inheriting class.
         **/
        virtual bool Destroy() = 0;

        /// Returns the sub-system initialization status.
        inline bool IsInit() const
        { return m_init; }

        /// Sets a custom output log.
        inline void SetLog(util::CLog& Log)
        { m_Log = Log; }

        /// Retrieves the subsystem name.
        inline const string_t& GetName() const
        { return m_name; }

        friend ZEN_API bool Init();
        friend ZEN_API void Quit();

    protected:
        /// No copying subsystems.
        CSubsystem(const CSubsystem&);/*            = delete;*/
        CSubsystem& operator=(const CSubsystem&);/* = delete;*/

        static std::vector<CSubsystem*> sp_allSystems;

        util::CLog& m_Log;
        string_t    m_name;
        bool        m_init;
    };
}   // namespace zen

#endif // ZENDERER__CORE__SUBSYSTEM_HPP

// Subsystem specification.

/**
 * @class zen::CSubsystem
 *
 * @details
 *  A valid engine subsystem can be initialized and destroyed without
 *  relying on the con/destructor. This is intentional so that errors can
 *  be handled gracefully by the caller, or internally if need be. A
 *  subsystem can accept a custom log (zen::util::CLog), found in
 *  Utilities/Log.hpp, but should preferably just be the global engine log
 *  that is created on engine initialization. If a custom log stream is
 *  provided (via `SetLog()`), any output will theoretically be logged
 *  to both the global engine log and the given stream, but this is
 *  implementation-specific to the derived classes. Most likely won't,
 *  largely due to exclusively to laziness and no desire to check
 *  `if(mp_Log != nullptr)` every single time something needs to be done.
 *
 *  As you can see in the examples below, a subsystem only returns `true` on
 *  a Destroy() call if it had been initialized successfully before. Also,
 *  you see that since a subsystem takes no parameters by default (a design
 *  limitation of C++ and inheritance in general), you can pass things to
 *  the constructor that the Init() method will use during actual
 *  initialization.
 *
 *  The CSubsystem constructor takes an optional string parameter that will
 *  give a name to the subsystem, which is useful for logging various
 *  actions later. This defaults to just "Subsystem".
 *
 * @example Subsystem
 *  @section bare_min A Bare Minimum Example
 *
 *  A valid subsystem could behave in a way similar to the example below.
 *  Let us assume that we have declared a derived class named `CRenderer`
 *  that takes a parameter for initialization and only implements the bare
 *  minimum methods implemented, as we will show here:
 *
 *  @code
 *  CRenderer::CRenderer(const int a) : CSubsystem("Renderer"), m_a(a) {}
 *
 *  bool CRenderer::Init()
 *  {
 *      if(m_a == 69)
 *          return (m_init = true);
 *      return false;
 *
 *      // Yay for one-liners:
 *      return (m_init = (m_a == 69));
 *  }
 *
 *  bool CRenderer::Destroy()
 *  {
 *      if(m_init)  return !(m_init = false);
 *      else        return false;
 *  }
 *  @endcode
 **/

/** @} **/
