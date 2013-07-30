/**
 * @file
 *  Zenderer/Utilities/Settings.hpp - An abstraction dictionary layer
 *  allowing easy setting of options.
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

#ifndef ZENDERER__UTILITIES__SETTINGS_HPP
#define ZENDERER__UTILITIES__SETTINGS_HPP

#include <string>
#include <map>

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Core/Subsystem.hpp"
#include "Zenderer/Utilities/INIParser.hpp"

namespace zen
{
namespace util
{
    /// Used internally by CSettings to manage options of all types.
    class ZEN_API COption
    {
    public:
        COption();
        COption(const string_t& value);
        COption(const COption& Opt);

        COption& operator=(const COption& Opt);     ///< Create from option
        COption& operator=(const string_t& name);   ///< Create from string
        COption& operator=(const char* name);       ///< Create from C-string
        COption& operator=(const bool name);        ///< Create from bool

        /// Create from numeric types.
        template<typename T>
        inline COption& operator=(const T& name)
        {
            m_value = std::to_string(name);
            return (*this);
        }

        /// Implicit conversion to `string_t`.
        inline operator string_t() const { return m_value; }

        /// Implicit conversion to `int`
        inline operator int() const { return atoi(m_value.c_str()); }

        /// Implicit conversion to `unsigned int`
        inline operator size_t() const { return atoi(m_value.c_str()); }

        /// Implicit conversion to `real_t`
        inline operator real_t() const { return atof(m_value.c_str()); }

        /// Implicit conversion to `bool`
        inline operator bool() const { return m_value != "0"; }

        /// Comparison to an option value.
        bool operator==(const COption& value)   const;
        bool operator==(const string_t& value)  const;  ///< @overload
        bool operator==(const bool value)       const;  ///< @overload

        template<typename T>
        inline bool operator==(const T& value) const
        { return (std::to_string(value) == m_value); }  ///< @overload

        /// Outputting an option value
        friend std::ostream& operator<<(std::ostream& o,
                                        const COption& Opt);

    private:
        string_t m_value;
    };

    /// An array-like wrapper for various settings.
    class ZEN_API CSettings : public CSubsystem
    {
        typedef std::map<
#ifdef ZEN_DEBUG_BUILD
            string_t,
#else
            uint32_t,
#endif // ZEN_DEBUG_BUILD
            COption> settings_t;

    public:
        CSettings(const string_t filename = "");
        ~CSettings();

        bool Init();
        bool Destroy();

        /**
         * Retrieves the value of an option (creates if non-existing).
         *  This will search for an option setting with the given name,
         *  and return it if it exists. If not, it will create an option
         *  with that name and return a reference to it, which would
         *  allow for a user to assign it immediately. This allows for
         *  the following:
         *  @code
         *  CSettings g_EngineSettings;
         *
         *  // Assigns a new option "FRAME_RATE" to the value 60.
         *  g_EngineSettings["FRAME_RATE"] = 60;
         *
         *  // Retrieves the option value "FRAME_RATE"
         *  std::cout << g_EngineSettings["FRAME_RATE"];
         *
         *  // Re-assigns the option "FRAME_RATE" to 24.
         *  g_EngineSettings["FRAME_RATE"] = 24;
         *  @endcode
         *
         *  This is possible by overloading `operator=` in an abstracted
         *  object COption that acts just like a normal type but can
         *  be assigned to an `int`, a `float`, a `bool`, or a `string_t`.
         *
         * @param   opt     Option to get value of
         *
         * @return  A mutable option value.
         **/
        COption& operator[](const string_t& opt);

        /// Starting iterator (for range-based `for()` loops).
        settings_t::iterator begin(){ return m_Options.begin(); }

        /// Ending iterator (for range-based `for()` loops).
        settings_t::iterator end()  { return m_Options.end(); }

        static inline CSettings& GetEngineInstance()
        {
            static CSettings Engine;
            return Engine;
        }

    private:
        CLog&       m_Log;
        settings_t  m_Options;
        string_t    m_filename;
    };
}
}

#endif // ZENDERER__UTILITIES__SETTINGS_HPP

/**
 * @class zen::util::CSettings
 * @details
 *  This isn't used anywhere in the engine, but is present for user
 *  convenience in creating / passing around various in-game settings
 *  that may arise. The link to the development blog contains info about the
 *  specific implementation, as well as potential usage functions.
 *
 * @see http://zenpandainteractive.blogspot.com/2013/06/setting-settings-of-arbitrary-types-in-c.html
 **/

/** @} **/
