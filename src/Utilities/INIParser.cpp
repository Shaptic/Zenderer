#include "Zenderer/Utilities/INIParser.hpp"

using namespace zen;

using util::LogMode;
using util::zParser;

string_t zParser::s_empty("");

zParser::zParser() : m_Log(util::zLog::GetEngineLog())
{
    m_pairs.clear();
}

zParser::~zParser()
{
    this->Reset();
}

bool zParser::LoadFromFile(const string_t& filename)
{
    m_Log.SetSystem("Parser");

    std::ifstream in(filename);
    if(!in)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << "Failed to open '" << filename << "'."
                << zLog::endl;

        return false;
    }

    return this->LoadFromStream(in, 0, -1, filename.c_str());
}

bool zParser::LoadFromStream(std::ifstream& file,
                             const std::streampos& start,
                             const std::streampos& finish,
                             const char* filename)
{
    ZEN_ASSERTM(file.good(), "bad file stream");

    file.seekg(start, std::ios::beg);

    string_t line;

    // For debugging.
    size_t line_no = 0;

    while(std::getline(file, line) && (finish == std::streampos(-1) ||
                                       file.tellg() < finish))
    {
        ++line_no;

        util::stripl(line);
        if(line.empty() || line[0] == '/' || line[0] == '<')
            continue;

        m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << "Parsing line " << line_no << " of '"
                << filename << "': " << line << zLog::endl;

        if(!this->ParseLine(line))
        {
            m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                    << "Malformed line in file stream (" << filename
                    << ")." << zLog::endl;
        }
    }

    file.seekg(start, std::ios::beg);
    return line_no > 0;
}

void zParser::Reset()
{
    m_pairs.clear();
}

bool zParser::Exists(const string_t& key) const
{
    return (m_pairs.find(key) == m_pairs.end());
}

const string_t& zParser::GetValue(const string_t& key) const
{
    auto find = m_pairs.find(key);

    if(find == m_pairs.end()) return s_empty;

    return find->second;
}

zParser::pair_t& zParser::GetPairs()
{
    return m_pairs;
}

std::streampos zParser::FindInFile(std::ifstream& stream,
                                   const string_t& finder,
                                   const std::streampos& max_pos)
{
    ZEN_ASSERTM(stream.good(), "bad file stream");

    string_t line;
    std::streampos start = stream.tellg();

    while(std::getline(stream, line) &&
         (max_pos == std::streampos(-1) || stream.tellg() < max_pos))
    {
        if(line.find(finder))
        {
            std::streampos ret = stream.tellg();
            stream.seekg(start, std::ios::beg);
            return ret;
        }
    }

    stream.seekg(start, std::ios::beg);
    return std::streampos(-1);
}

bool zParser::ParseLine(const string_t& line)
{
    util::stripl(line);
    util::stript(line);

    std::vector<string_t> pair = util::split(line, '=');

    if(pair.size() != 2) return false;

    m_pairs[pair[0]] = pair[1];
    return true;
}
