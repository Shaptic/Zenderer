/**
 * @file
 *  Zenderer/Utilities/Log.hpp - A suite for logging engine info to a file.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.1
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

#ifndef ZENDERER__UTILITIES__LOG_HPP
#define ZENDERER__UTILITIES__LOG_HPP

// For storing the date at which the log opened
#include <ctime>

// File and string streams
#include <fstream>
#include <sstream>
#include <string>

// Screen output
#include <iostream>

#include "Zenderer/Core/ErrorHandling.hpp"
#include "Zenderer/Core/Types.hpp"

namespace zen
{

namespace gfx
{
    /// Forward declaration for true definition in Zenderer/Scene.hpp.
    void error_window(const char* msg, const char* cap,
                      const uint16_t w = 300, const uint16_t h = 200);
}

namespace util
{
    /// Log output types
    enum class ZEN_API LogMode
    {
        ZEN_DEBUG,  ///< Only in builds with the ZEN_DEBUG_BUILD flag.
        ZEN_INFO,   ///< Standard message.
        ZEN_ERROR,  ///< Worrisome but handleable error.
        ZEN_FATAL   ///< Critical error causing immediate termination.
    };

    /// Creates a special file stream to store logging information.
    class ZEN_API zLog
    {
    public:
        /**
         * Sets internal variables to a known state.
         *  This constructor does next to nothing. The filename that is
         *  passed will not be used to create a file until Init() is
         *  called. It acts exactly like a zen::zSubsystem instance,
         *  but cannot actually be one due to the fact that that class
         *  uses a log within it.
         *
         * @param   filename    Filename for output log
         * @param   show_stdout Output logged info to the screen?
         *
         * @see     zLog::Init()
         * @see     zLog::ToggleStdout()
         **/
        zLog(const string_t& filename, const bool show_stdout =
#ifdef ZEN_DEBUG_BUILD
                true);
#else
                false);
#endif // ZEN_DEBUG_BUILD

        // Deleting member functions is not implemented in VS2012.

        /// Copy constructor does not exist.
        zLog(const zLog& Copy)/* = delete*/ {}
        zLog& operator=(const zLog& Copy)/* = delete;*/ { return *this; }

        /**
         * This destructor does nothing but call Destroy().
         *  The zen::zSubsystem specification requires that the destructor
         *  only call Destroy(), and this behaves in a similar fashion.
         *
         * @see     zen::zSubsystem::~zSubsystem()
         **/
        virtual ~zLog();

        /**
         * Executes a zLog method in-stream.
         *  This allows for chaining zLog method calls in `operator<<` with
         *  the guarantee that they will be executed in the specified
         *  order.
         *  Thus, we can inline the new-line method call like so:
         *
         *      Log << Log.SetSystem("Test") << "Data" << zLog::endl;
         *
         *  And it will function identically.
         *
         * @param   method  zLog method function
         *
         * @return  A reference to oneself, to allow for chaining.
         *
         * @note    The passed method must return a zLog& instance, and must
         *          take no parameters.
         *
         * @note    This should only be called through the zLog::endl pointer.
         *
         * @see     zLog::endl
         **/
        virtual inline zLog& operator<< (zLog& (zLog::*method)()); // Dat syntax

        /**
         * Logs given data.
         *  Due to its templated nature, and the versatility of
         *  `std::ostream`, almost anything can be passed as an argument.
         *  The given data is written to an `std::stringstream`, and is
         *  only writted to the file when zLog::endl is called.
         *
         * @param   data    Data to log
         *
         * @return  A reference to itself, to 'daisy-chain' statements.
         **/
        template<typename T>
        zLog& operator<< (const T& data);

        /**
         * Opens a file stream with a custom filename given in the ctor.
         *  This method accepts no parameters because it is intended to
         *  behave similarly to zen::zSubsystem. Thus, any parameters to
         *  Init() are typically handled in the constructor.
         *
         * @return  `true`  if the file opened successfully, and
         *          `false` on any error.
         *
         * @see     Destroy()
         **/
        virtual bool Init();

        /**
         * Writes out any leftover data and closes the file stream.
         *  After calling this method, it is possible to re-open a new log
         *  file by calling SetFilename() and subsequently calling Init()
         *  again.
         *  This should not be called on the global engine log, because
         *  then messages logged after zen::Quit() will not be recorded.
         *
         * @return  `true`  if Init() had called successfully, and
         *          `false` if not.
         **/
        virtual bool Destroy();

        /// Toggles console output.
        void ToggleStdout();

        /// Disables logging.
        inline void Disable();

        /// Enables logging.
        inline void Enable();

        /// Sets the output mode to something in util::LogMode.
        virtual inline zLog& SetMode(const LogMode& Mode);

        /// Sets the current logging subsystem.
        virtual inline zLog& SetSystem(const string_t& sys);

        /// Sets the log filename for a call to Init() after Destroy().
        inline void SetFilename(const string_t& fn);

        /// Retrieves the current logging system.
        inline const string_t& GetSystem() const;

        /// Retrieves the current log filename.
        inline string_t GetFilename() const;

        /// Returns the sub-system initialization status.
        inline bool IsInit() const;

        /// Makes it possible to inline the `Set*()` methods.
        friend inline std::ostream&
        operator<<(std::ostream& o, const zLog& Log) { return o; }

        /// Singleton access to the engine log.
        static zLog& GetEngineLog();

        /// Shortcut to the new line method pointer.
        static zLog& (zLog::*endl)();

    private:
        /// Writes stream to the file and moves to the next line.
        virtual zLog& Newline();

        std::stringstream   m_str;
        std::ofstream       m_log;

        LogMode     m_mode;
        string_t    m_system;
        string_t    m_filename;
        bool        m_stdout;
        bool        m_init;
        bool        m_enabled;
    };

    // Include definition for the templated operator<<
    #include "Log.inl"
}   // namespace util
}   // namespace ic

#endif // ZENDERER__UTILITIES__LOG_HPP

// Detailed documentation for the above class.

/**
 * @class zen::util::zLog
 * @details
 *  If desirable, it can be inherited to provide a dummy logger to
 *  the engine for speed or whatever other reason.
 *
 *  By default, one of these is created for the engine log, but more
 *  streams can be opened as needed.
 *
 *  The stream immediately writes to the file. If specified (via a
 *  constructor or ToggleStdout()), the data will be immediately
 *  output to the screen, as well.
 *  This eliminates the need to flush the stream repeatedly and output
 *  data in chunks, as was the case in @a IronClad.
 *
 *  Log output is formatted like so:\n
 *  <pre>
 *  [MODE] Subsystem -- data
 *  </pre>
 *
 *  MODE is one of the 4 output modes in the util::LogMode enumerator:  \n
 *  <pre>
 *      DEBUG   --  Only shows up in debug builds (ZEN_DEBUG_BUILD directive)
 *      INFO    --  Generic information
 *      ERROR   --  Error that should cause alarm but no serious harm
 *      FATAL   --  Critical error that terminates the program
 *                  with the given data as the message in the pop-up
 *                  window.
 *  </pre>
 *
 *  Logging works by calling SetMode(), which will be output for every
 *  subsequent line until SetMode() is called again. Newlines are not
 *  automatically detected, so you must force them via zLog::endl.
 *
 *  You should specify the subsystem that is doing the
 *  logging via SetSystem(), it will default to "Log".
 *
 *  You can directly stream SetMode() and SetSystem() as output, like so:
 *  @code
 *  using util::zLog;
 *  using util::LogMode;
 *
 *  zLog Output("Output.log", false);
 *  Output.Init();
 *
 *  Output << Output.SetMode(LogMode::ZEN_FATAL) <<
 *         << Output.SetSystem("Game")  << "Failure." << zLog:endl;
 *
 *  Output.Destroy();
 *  @endcode
 *
 *  The output log (in `Output.log`) would look like this:
 *  <pre>
 *  [FATAL] Game -- Failure.
 *  </pre>
 *  In addition, a popup MessageBox (on Windows) will show with the caption
 *  "Game" and text saying "Failure." due to the specification of `ZEN_FATAL`.
 *
 *  You can see a few usage scenarios in the [Examples](examples.html)
 *  section.
 *
 * @example Logging
 *  @section Usage
 *
 *  Here are some various usage scenarios:
 *  @code
 *  // Using proper line breaks.
 *  string_t name = "halcyon";
 *  zLog Output("Output.log", true);
 *  Output.Init();  // Error checking obviously omitted.
 *  Output.SetMode(LogMode::ZEN_INFO);
 *
 *  Output << "Hi " << name << ", I'm a log." << zLog::endl
 *         << "This is the second line." << zLog::endl;
 *
 *  // This happens if you forget them, or do them by hand via \n:
 *  Output.SetSystem("Example");
 *  Output.SetMode(LogMode::ZEN_DEBUG) << "Random data\n";
 *  Output << "More data on the next line.";
 *  Output.SetMode(LogMode::ZEN_ERROR) << Output.SetSystem("Test")
 *                                     << "And a third line." << zLog::endl;
 *  Output.Destroy();
 *  @endcode
 *
 *  The resulting output would look like this:
 *  <pre>
 *  [INFO ]  Log -- Hi halcyon, I'm a log.
 *  [INFO ]  Log -- This is the second line.
 *  [ERROR]  Test -- Random data
 *  More data on the next line.And a third line.
 *  </pre>
 *
 *  As you can see, it's critical to stream `zLog::endl` when you've
 *  outputted a line for proper formatting. It *must* be called prior
 *  to switching modes or systems via `SetMode()` and `SetSystem()`,
 *  respectively. Otherwise, only the latest changes will actually be
 *  output.
 **/

/** @} **/
