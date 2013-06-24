/**
 * @file
 *  Zenderer/Utilities/INIParser.hpp - A `key=value` pair file parser used
 *  throughout @a Zenderer to load various things.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0.2
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

#ifndef ZENDERER__UTILITIES__INI_PARSER_HPP
#define ZENDERER__UTILITIES__INI_PARSER_HPP

#include <fstream>
#include <sstream>
#include <map>

#include "Zenderer/Core/Types.hpp"

#include "Log.hpp"
#include "Assert.hpp"
#include "Helpers.hpp"

namespace zen
{
namespace util
{
    /// Parses files based on key=value1,...,valueN pairs, like in `.ini` files.
    class ZEN_API CINIParser
    {
    public:
        CINIParser();
        virtual ~CINIParser();

        typedef std::map<string_t, string_t> pair_t;

        /**
         * Fills the dictionary with key=value pairs for the entire file.
         * @param   string_t    Filename to parse
         * @return  `true` if file was parsed successfully
         *          `false` otherwise.
         **/
        bool LoadFromFile(const string_t& filename);

        /**
         * Fills the key-value dictionary by parsing a file stream.
         *  This is more versatile than the generic LoadFromFile(), since
         *  you can specify where in the stream you would like to start
         *  and where you would like to end. By default, the whole file is
         *  read.
         *  This comes in handy if a file, for example, has several entity
         *  specifications, so there's more than one "texture" key. Thus
         *  you could read until "<entity>", mark that, read till
         *  "</entity>", mark that, then pass the stream to this method
         *  and it would only give you the key-value pairs found in that
         *  section of the file.
         *
         * @param   std::ifstream   File stream to read
         * @param   std::streampos  Starting point in the stream    (opt=0)
         * @param   std::streampos  Ending point in the stream      (opt=END)
         * @param   char*           Filename the stream comes from  (opt="file")
         *
         * @return  `true` if at least one pair was parsed,
         *          `false` otherwise, or if the given filestream was not open.
         **/
        bool LoadFromStream(std::ifstream& file,
                            const std::streampos& start = 0,
                            const std::streampos& finish= -1,
                            const char* filename = "file");
        bool LoadFromStream(const char**    str,
                            const uint32_t  start,
                            const uint32_t  finish);

        /// Clears the dictionary.
        void Reset();

        /**
         * Retrieves a value based on a key in the dictionary.
         *  If the file stream contained `texture=Data.tga`, and you
         *  called GetValue("texture"), the function gives back "Data.tga".
         *
         * @param   std::string     Key to access
         *
         * @return  Value if it exists, empty string otherwise.
         **/
        std::string GetValue(const string_t& key) const;

        /// Attempts to return an integer representation of a value.
        /// @warning No error checking is performed on conversion.
        inline int GetValuei(const string_t& key) const;

        /// Attempts to return a boolean representation of a value.
        /// @warning No error checking is performed on conversion.
        inline bool GetValueb(const string_t& key) const;

        /// Attempts to return a floating point representation of a value.
        /// @warning No error checking is performed on conversion.
        inline float GetValuef(const string_t& key) const;

        /**
         * Returns an array of values from a key-value pair based on a char.
         *  This will split the 'value' part of a pair based on a
         *  delimiter, and return the result.
         *
         * @param   std::string     Key to access
         * @param   char            Character to split 'value' on
         *
         * @return  Result if 'key' exists,
         *          the 'value' in the pair (in a `vector`) if not.
         *
         * @see     zen::util::split()
         **/
        std::vector<string_t> GetValues(const string_t& key,
                                        const char delimiter = ',') const;

        pair_t& GetPairs();                 ///< Direct access to the dictionary.
        const pair_t& GetPairs() const;     ///< Direct immutable access to the dictionary.

        /**
         * Finds the first instance of a string within a file stream.
         *  This function basically reads a file until it finds an instance
         *  of `finder` in the current line OR up until it reaches the
         *  optional `max_pos` argument, which specifies a position in
         *  the stream to stop at, whichever comes first.
         *  If `max_pos` is left at the default, -1, the search
         *  is done till EOF (or some other reason causing `std::getline`
         *  to return `false`).
         *
         *  The function will return the stream to its starting point
         *  as it was when the function was called.
         *
         * @param   std::ifstream&  Stream to search
         * @param   string_t        String to find
         * @param   std::streampos  Place to stop search (optional=`EOF`)
         *
         * @return  Position the requested string was found at,
         *          `std::streampos(-1)` if it was not found.
         *
         * @note    Stream is left at the same point it started at.
         **/
        static std::streampos FindInFile(std::ifstream& stream,
                                         const string_t& finder,
                                         const std::streampos& max_pos = -1);

    private:
        bool ParseLine(const string_t& line);

        // An empty string, so GetValue() doesn't return a temporary
        // buffer when nothing is found in the 'const' version.
        static string_t s_empty;

        CLog& m_Log;
        pair_t m_pairs;
    };

    #include "INIParser.inl"
}   // namespace util
}   // namespace zen

#endif // ZENDERER__UTILITIES__INI_PARSER_HPP

/**
 * @class zen::util::CINIParser
 *
 * @description
 *  This file parser defines a very generic type of parsing, since most
 *  files (meshes, levels) in @a Zenderer use a similar formatting.
 *  For example, in a ZenLevel (see specs/ZenLevel) , an entity can be
 *  placed like so:
 *
 *      <entity>
 *          texture=Quad.tga
 *          position=100,200
 *      </entity>
 *
 *  So you can see that values are split up by an '=' delimiter. Thus,
 *  this generic parser can create a dictionary of values that can
 *  then be easily accessed.
 *  This eliminates the need to read the file sequentially and worry
 *  about what might come later and what options are defined while
 *  you're reading.
 *  Now, you can read in the file (or portion of a file), and then
 *  access the values by key, given that they exist.
 *  The parser will complete ignore lines beginning with the forward
 *  slash ('/') character (after stripping leading whitespace) and the
 *  less-than character ('<'), since these are typically tags ("<likethis>")
 *  and comments ("// like this").
 **/

/**
 * @fn zen::util::CINIParser::GetValues
 *
 * @example Parsing Multiple Values
 *  @section Usage
 *
 *  Let's assume we have a file whose contents look like so:
 *
 *  <pre>
 *  file=example.txt
 *  author=me
 *  package=zenderer
 *
 *  // here is the point:
 *  values1=stuff,morestuff,finalstuff
 *  values2=data:moredata:finaldata
 *  </pre>
 *
 *  And we want to retrieve the values after `values1` and
 *  `values2` separately, since zen::util::CINIParser::GetValue()
 *  with `"values1"` as an argument would return
 *  `"stuff,morestuff,finalstuff"`. We can thus use
 *  zen::util::CINIParser::GetValues() to achieve the effect we
 *  want, like so:
 *
 *  @code
 *      zen::util::CINIParser Parser;
 *      Parser.LoadFromFile("example.txt");
 *
 *      // Delimited defaults to a comma (',') character
 *      std::vector<string_t> keyValues = Parser.GetValues("values1");
 *
 *      for(size_t i = 0; i < keyValues.size(); ++i)
 *      {
 *          std::cout << keyValues[i] << ' ';
 *      }
 *
 *      keyValues = Parser.GetValues("values2", ':');
 *      for(size_t i = 0; i < keyValues.size(); ++i)
 *      {
 *          std::cout << keyValues[i] << ' ';
 *      }
 *  @endcode
 *
 *  The result would be:
 *
 *  @code
 *      stuff morestuff finalstuff data moredata finaldata
 *  @endcode
 **/

/** @ **/
