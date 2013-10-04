#include "Zenderer/Utilities/FileParser.hpp"

using zen::string_t;
using namespace zen::util;

zFileParser::~zFileParser()
{
    m_results.clear();
}

bool zFileParser::LoadFromFile(const string_t& filename)
{
    ZEN_ASSERT(!filename.empty());

    std::ifstream infile(filename);
    return this->LoadFromStream(infile, 0, -1, filename.c_str());
}

bool zFileParser::LoadFromStream(std::ifstream& infile,
                                 const std::streampos start,
                                 const std::streampos end,
                                 const char* const fname,
                                 const bool skip)
{
    m_results.clear();
    infile.seekg(start, std::ios::beg);
    std::string line;

    if(!infile || (end != std::streampos(-1) && start >= end)) return false;

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
                line.substr(index + 1)
            ));
        }
    }

    if(!skip) infile.seekg(start, std::ios::beg);
    return true;
}

bool zFileParser::LoadFromStreamUntil(std::ifstream& infile,
                                      const string_t& end,
                                      const std::streampos start,
                                      const char* const filename,
                                      const bool skip)
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
                line.substr(index + 1)
            ));
        }
    }

    if(!skip) infile.seekg(start, std::ios::beg);
    return true;
}

string_t zFileParser::PopResult(const string_t& index, string_t def)
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

string_t zFileParser::GetFirstResult(const string_t& index,
                                     const string_t& def) const
{
    auto it = std::find_if(m_results.begin(), m_results.end(),
        [&index](decltype(*m_results.cbegin())& result) -> bool
        {
            return result.first == index;
        }
    );

    return it == m_results.end() ? def : it->second;
}

std::vector<string_t> zFileParser::GetResults(const string_t& index) const
{
    std::vector<string_t> results;
    for(auto& i : m_results)
        if(index == i.first)
            results.push_back(i.second);

    return results;
}
