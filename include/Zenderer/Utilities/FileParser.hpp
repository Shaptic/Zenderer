/**
 * @file
 *  Zenderer/Utilities/FileParser.hpp - A parser for the deformed child of
 *  an XML-INI orgy that is used throughout @a Zenderer for describing data.
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

#ifndef ZENDERER__UTILITIES__FILE_PARSER_HPP
#define ZENDERER__UTILITIES__FILE_PARSER_HPP

#include <vector>
#include <fstream>
#include <utility>
#include <algorithm>

#include "Zenderer/Core/Types.hpp"
#include "Assert.hpp"

namespace zen
{
namespace util
{
    /// @a Zenderer-specific file format parser.
    class zFileParser
    {
        typedef std::pair<string_t, string_t> pair_t;

    public:
        zFileParser();
        ~zFileParser();
        
        /**
         * Parses an entire file according to the format specification.
         *
         * @param   filename    Path to a file to parse
         *
         * @return  `true`  if the file was parsed successfully,
         *          `false` otherwise.
         *
         * @see     GetError()
         **/
        virtual bool LoadFromFile(const string_t& filename)
        {
            ZEN_ASSERT(!filename.empty());
            
            std::ifstream infile(filename);
            return this->LoadFromStream(infile, 0, 0, filename.c_str());
        }
        
        /**
         * Parses a portion of a filestream.
         *  This method is made to parse chunks of a file at a time, in order
         *  to avoid confusion of various pairs that may be related to a 
         *  different part of the file.
         *  You can provide optional starting and ending stream positions, 
         *  which default to the beginning and end of the file, respectively.
         *
         * @param   file    File stream to parse on
         * @param   start   Stream position in the file to begin parsing
         * @param   end     Stream position in the file to end parsing
         * @param   fname   Filename of the stream, if applicable
         *
         * @post    The internal file result storage is filled with parsed
         *          data. Regardless of whether or not there was an error,
         *          all old parser state is lost.
         *
         * @return  `true`  if the file was parsed up to `end`,
         *          `false` otherwise. Potential reasons for failure include
         *                  a bad file stream, a bad range of positions, or
         *                  a fatal parsing error.
         **/
        virtual bool LoadFromStream(std::ifstream& infile,
                                    const std::streampos start = 0,
                                    const std::streampos end = -1,
                                    const char* const fname = nullptr)
        {
            m_results.clear();
            infile.seekg(start, std::ios::beg);
            std::string line;
            
            if(!infile || (end != -1 && start >= end)) return false;
            
            while((end == std::streampos(-1) || infile.tellg() < end) && 
                  std::getline(infile, line))
            {
                util::strip(line);

                // Skip empty lines, comments.
                if(line.empty() ||
                  (line.size() > 2 && line.substr(0, 2) == "//"))
                    continue;
                
                std::size_t index = line.find('=');
                if(index != std::string::npos)
                {
                    m_results.emplace_back(pair_t(
                        line.substr(0, index),
                        line.substr(index)
                    ));
                }
            }
            
            infile.seekg(start, std::ios::beg);
            return true;
        }
        
        /**
         * Parses a stream until reaching a sequence.
         *  This is useful to the internal @a Zenderer file loaders because
         *  it allows for the parser to return a chunk of results for an
         *  XML-like block in the file. For example, upon reaching an 
         *  &lt;enemy&gt; tag, you could do
         *  `LoadFromStreamUntil(stream, "&lt;/enemy&gt;", stream.tellg())`
         *  in order to capture all set parameters for that specific enemy
         *  descriptor.
         *
         * @param   file    File stream to parse on
         * @param   end     String sequence to quit parsing on if found
         * @param   start   Stream position in the file to start parsing
         * @param   fname   Filename of the stream, if applicable
         *
         * @post    The internal file result storage is filled with parsed
         *          data. Regardless of whether or not there was an error,
         *          all old parser state is lost.
         *
         * @return  `true`  if the file was parsed up to `end`,
         *          `false` otherwise. Potential reasons for failure include
         *                  a bad file stream, a bad range of positions, or
         *                  a fatal parsing error.
         **/
        virtual bool LoadFromStreamUntil(std::ifstream& infile, 
                                         const string_t& end,
                                         const std::streampos start = 0,
                                         const char* const filename = nullptr)
        {
            m_results.clear();
            infile.seekg(start, std::ios::beg);
            std::string line;
            
            if(!infile) return false;
            
            while(std::getline(infile, line) &&
                  line.find(end) == std::string::npos)
            {
                util::strip(line);

                // Skip empty lines, comments.
                if(line.empty() ||
                  (line.size() > 2 && line.substr(0, 2) == "//"))
                    continue;
                
                std::size_t index = line.find('=');
                if(index != std::string::npos)
                {
                    m_results.emplace_back(pair_t(
                        line.substr(0, index),
                        line.substr(index)
                    ));
                }
            }
            
            infile.seekg(start, std::ios::beg);
            return true;
        }
        
        /**
         * Pop a result off of the container and return it.
         *
         * @param   index   Index to search results for
         * @param   def     Default value if `index` isn't found
         *
         * @return  The matching value for the index if it's found,
         *          and the default string otherwise.
         **/
        string_t PopResult(const string_t& index, string_t def = "")
        {
            for(auto i = m_results.begin(), 
                     j = m_results.end(); 
                     i != j; ++i)
            {
                auto it = *i;
                if(it.first == index)
                {
                    def = it.second;
                    m_results.erase(i);
                    break;
                }
            }
            
            return def;
        }
        
        /// Checks if an index exists in the results.
        inline bool Exists(const string_t& index) const
        {
            return !GetFirstResult(index).empty();
        }
        
        /**
         * Retrieves the first result for an index.
         *  This will leave the result untouched, so multiple calls to this
         *  method will continue to return the same values.
         *
         * @param   index   Index to search a value for
         * @param   def     Default to return if no result is found
         *
         * @return  The first result matching `index`, and `def` otherwise.
         **/
        string_t GetFirstResult(const string_t& index, const string_t& def = "") const
        {
            auto it = std::find_if(m_results.begin(), m_results.end(),
                [&index](decltype(*m_results.cbegin())& result) -> bool
                {
                    return result.first == index;
                }
            );
            
            return it == m_results.end() ? def : it->second;
        }
        
        /**
         * Retrieves all results for an index.
         *  This will leave the results untouched, so multiple calls to this
         *  method will continue to return the same values.
         *
         * @param   index   Index to search a value for
         * @param   def     Default to return if no result is found
         *
         * @return  A container with all results for the given `index`.
         **/
        std::vector<string_t> GetResults(const string_t& index) const
        {
            std::vector<string_t> results;
            for(auto& i : m_results)
                if(index == i.first)
                    results.push_back(i.second);

            return results;
        }
        
        /// Returns the total number of results for all index values.
        inline size_t GetResultCount() const { return m_results.size(); }

    private:
        std::vector<pair_t> m_results;
    };
}
}

#endif // ZENDERER__UTILITIES__FILE_PARSER_HPP

/** @} **/
