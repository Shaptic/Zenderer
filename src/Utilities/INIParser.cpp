#include "Zenderer/Utilities/INIParser.hpp"

using namespace zen;

USING_ZENDERER_LOG;

using util::CINIParser;

CINIParser::CINIParser() 
{
    m_pairs.clear();
}

CINIParser::~CINIParser()
{
    this->Reset();
}

bool CINIParser::LoadFromFile(const string_t& filename)
{
    g_EngineLog.SetSystem("Parser");
    
    std::ifstream in(filename);
    if(!in)
    {
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                    << "Failed to open '" << filename << "'."
                    << CLog::endl;

        return false;
    }
    
    string_t line;
    size_t line_no = 0;
    
    while(std::getline(in, line))
    {
        ++line_no;
        
        util::stripl(line);
        if(line.empty() || line[0] == '/' || line[0] == '<')
            continue;
            
        if(!this->ParseLine(line))
        {
            g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                        << "Malformed line on line " << line_no
                        << " of '" << filename << "'." << CLog::endl;
        }
    }
    
    return (line_no > 0);
}

bool CINIParser::LoadFromStream(std::ifstream& file, 
                             const std::streampos& start,
                             const std::streampos& finish,
                             const char* filename)
{
    ZEN_ASSERTM(file, "bad file stream");
    
    file.seekg(start, std::ios::beg);
    
    string_t line;
    
#ifdef _DEBUG
    size_t line_no = 0;
#endif // _DEBUG
    
    while(std::getline(file, line) && file.tellg() < finish)
    {
#ifdef _DEBUG
        ++line_no;
#endif // _DEBUG

        util::stripl(line);
        if(line.empty() || line[0] == '/' || line[0] == '<')
            continue;
        
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_DEBUG)
                    << "Parsing line " << line_no << " of '"
                    << filename << "': " << line << CLog::endl;

        if(!this->ParseLine(line))
        {
            g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                              << "Malformed line in file stream ("
                              << filename << ")." << CLog::endl;
        }
    }
    
    file.seekg(start, std::ios::beg);
    return (!file.bad());
}

void CINIParser::Reset()
{
    m_pairs.clear();
}

CINIParser::pair_t& CINIParser::GetPairs()
{
    return m_pairs;
}

std::streampos CINIParser::FindInFile(std::ifstream& stream,
                                   const string_t& finder,
                                   const std::streampos& max_pos)
{
    ZEN_ASSERTM(stream, "bad file stream");
    
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

bool CINIParser::ParseLine(const string_t& line)
{
    util::stripl(line);
    util::stript(line);
    
    std::vector<string_t> pair = util::split(line, '=');
    
    if(pair.size() != 2) return false;
    
    m_pairs[pair[0]] = pair[1];
    return true;
}
