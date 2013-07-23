/**
 * @file
 *  Zenderer/Utilities/XMLParser.hpp - A minimalistic XML parsing utility.
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

#include "Zenderer/Core/Types.hpp"

#include "Log.hpp"
#include "Assert.hpp"
#include "Helpers.hpp"

// Necessary include files
#include <vector>
#include <map>
#include <fstream>
#include <functional>

// Declare namespaces if we're a part of Zenderer.
namespace zen
{
namespace util
{
    // Forward declaration.
    struct ZEN_API XMLNode;

    /// Shortcut for a list of nodes.
    typedef std::vector<XMLNode*> XMLList_t;

    /// Shortcut for an option dictionary.
    typedef std::map<string_t, string_t> XMLOptions_t;

    /**
     * A single XML node.
     *
     * Given:
     *
     * @code{.xml}
     * <!-- Sample XML file -->
     * <html>
     *      <body background="nothing">
     *          Some text
     *      </body>
     * </html>
     * @endcode
     *
     * You will get:
     *
     *  Member      |   Data                        |
     *  :---------- | :-----------------------------|
     *  parent      | Pointer to the HTML node
     *  children    | An empty `std::vector`
     *  options     | `["background": "nothing"]`
     *  content     | `"Some text"`
     *  name        | `"body"`
     **/
    struct ZEN_API XMLNode
    {
        XMLNode*        parent;     ///< Parent node
        XMLList_t       children;   ///< List of child nodes
        XMLOptions_t    options;    ///< List of node options
        string_t        content;    ///< Text of node
        string_t        name;       ///< Name of the node
    };

    /// A minimalistic XML parser that expects near-perfection from the user.
    /// @todo   Fix `XMLNode` parent setting
    class ZEN_API CXMLParser
    {
    public:
        /// Does nothing.
        CXMLParser();

        /// Clears the internal XML tree.
        ~CXMLParser();

        /// Loads and parses an XML file from disk.
        bool LoadFromFile(const string_t& filename);

        /// Loads and parses XML data from a string.
        bool LoadFromString(const char** str);

        /// Retrieve the immutable node data for the first tag occurrence.
        const XMLNode* const FindNode(const string_t& name) const;

        /// Retrieve the mutable node data for the first tag occurrence.
        XMLNode* FindNode(const string_t& name);

        /// Deletes the XML tree.
        void ClearTree();

        /// Returns iterator to root of tree (for range-based `for()` loops).
        inline XMLList_t::iterator begin();
        inline const XMLList_t::const_iterator cbegin() const;

        /// Returns ending iterator of tree (for range-based `for()` loops).
        inline XMLList_t::iterator end();
        inline const XMLList_t::const_iterator cend() const;

        /**
         * Outputs an XML error.
         *  With @a Zenderer present, this will output to the engine
         *  log as a non-fatal error.
         *  Output format:
         *
         *  [ERROR] XML Parser -- Malformed XML on line `line_no`: `line` (`reason`)
         *
         * @param   line_no Line that the error occurred on
         * @param   line    The actual XML line
         * @param   reason  The reason for the error
         **/
        static void ShowXMLError(const uint32_t line_no,
                                 const string_t& line,
                                 const string_t& reason);

    private:
        /// Recursive method to parse an XML tree.
        int ParseTree(const char** str, const int index, XMLNode* parent);
        int ParseTree(std::ifstream& f, const int index, XMLNode* parent);

        int CreateNode(const string_t& line, XMLNode* parent, int i,
                       std::function<int(const size_t, XMLNode*)> callback);

        XMLList_t m_XMLTree;
        CLog& m_Log;
    };

    XMLList_t::iterator CXMLParser::begin()
    {
        return m_XMLTree.begin();
    }

    const XMLList_t::const_iterator CXMLParser::cbegin() const
    {
        return m_XMLTree.cbegin();
    }

    XMLList_t::iterator CXMLParser::end()
    {
        return m_XMLTree.end();
    }

    const XMLList_t::const_iterator CXMLParser::cend() const
    {
        return m_XMLTree.cend();
    }
}   // namespace util
}   // namespace zen

// Detailed Doxygen documentation.

/**
 * @class   zen::util::CXMLParser
 *
 * @details
 *  This parser is definitely not W3C compliant and will only accept
 *  a very strict subset of standard XML code.
 *
 *  @section Format
 *
 *  @subsection Tags
 *      Tags can only be expressed on a single line; the parser will refuse
 *      to parse the remainder of the file if it finds an unclosed tag.
 *      Tag names cannot contain spaces; well, theoretically, they can,
 *      but it's undefined behavior since I haven't tested it.
 *      Tags must be on lines by themselves, as well as their closing tags.
 *      Tags without a body are not allowed.
 *      There must be an equal number of opening and closing tags, but
 *      they don't have to necessarily have identical names :)
 *      meaning
 *
 *          <open>
 *              stuff
 *          </close>
 *
 *      is perfectly valid as far as this parser is concerned!
 *
 *      @code{.xml}
 *      <!-- Examples of invalid tags -->
 *      <sometag
 *      >
 *      </sometag>
 *      <no spaces>
 *      </no spaces>
 *      <closeOnOneLine>    </closeOnOneLine>
 *      <bodyless_tag/>
 *      <!-- Nameless tag -->
 *      <>
 *      <tag1>
 *      <!-- EOF without an end to tag1 -->
 *      @endcode
 *
 *  @subsection Comments
 *      Comments can only span a single line. They are distinguished by
 *      a `&lt;!-- ` prefix and end with a ` --&gt;` suffix. The spaces
 *      *are required*.
 *
 *      @code{.xml}
 *      <!-- Examples of invalid comments -->
 *      <!-- No closing tag
 *      <!--No space after prefix -->
 *      <!-- No space before suffix-->
 *      <!--Nospacesatall-->
 *      @endcode
 *
 *  @subsection Options
 *      Options must be separated only by single spaces and the option
 *      values must be encompassed in double-quotations. There must not
 *      be anything between the option name and its value (other than the
 *      `=` character, of course).
 *
 *      @code{.xml}
 *      <!-- The perfect XML tag with options: -->
 *      <best_tag option1="the best" option2="tag ever">
 *      </best_tag>
 *
 *      <!-- Examples of invalid options -->
 *
 *      <!-- no quotes around values -->
 *      <tag opt1=2 opt2=3>
 *
 *      <!-- Not separated by a single space -->
 *      <tag opt1="2"    opt2="3">
 *
 *      <!-- Space after last option -->
 *      <tag opt1="2" opt2="3" >
 *
 *      <!-- Spaces between the '=' character -->
 *      <tag opt1 = "2" opt2     ="3">
 *      @endcode
 *
 *  In summation: stay perfect. If you think it might not work, it
 *  probably wont :)
 **/

/** @} **/
