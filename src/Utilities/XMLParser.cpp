#include "Zenderer/Utilities/XMLParser.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

using util::zXMLParser;

zXMLParser::zXMLParser() : m_Log(zLog::GetEngineLog())
{
    m_XMLTree.clear();
}

zXMLParser::~zXMLParser()
{
    this->ClearTree();
}

bool zXMLParser::LoadFromFile(const string_t& filename)
{
    std::ifstream file(filename);

    if(!file)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("XML Parser")
                << "Failed to open '" << filename << "'."
                << zLog::endl;

        return false;
    }

    this->ClearTree();
    int retval = this->ParseTree(file, 0, nullptr);
    return (retval == 0);
}

bool zXMLParser::LoadFromString(const char** data)
{
    if(data != nullptr) this->ClearTree();
    return (this->ParseTree(data, 0, nullptr) == 0);
}

void zXMLParser::ClearTree()
{
    auto i = m_XMLTree.begin();

    while(i != m_XMLTree.end())
    {
        delete *i;
        i = m_XMLTree.erase(i);
    }

    m_XMLTree.clear();
}

void zXMLParser::ShowXMLError(const uint32_t line_no, const string_t& line,
                              const string_t& reason)
{
    zLog& Log = zLog::GetEngineLog();

    Log << Log.SetMode(LogMode::ZEN_ERROR)
        << Log.SetSystem("XML Parser")
        << "Malformed XML on line " << line_no
        << ": " << line << " (" << reason << ") "
        << zLog::endl;
}

/// @see zen::asset::zAssetManager::Find
const util::XMLNode* const zXMLParser::FindNode(const string_t& name) const
{
    // Fastest way to loop (see Zenderer docs).
    auto i = m_XMLTree.cbegin(), j = m_XMLTree.cend();

    // Hashed string comparisons in Zenderer.
    uint32_t hash = util::string_hash(name);

    for( ; i != j; ++i)
    {
        uint32_t comp = util::string_hash((*i)->name);
        if(comp == hash) return (*i);
    }

    return nullptr;
}

// Identical to non-const.
util::XMLNode* zXMLParser::FindNode(const string_t& name)
{
    // Fastest way to loop (see Zenderer docs).
    auto i = m_XMLTree.cbegin(), j = m_XMLTree.cend();

    // Hashed string comparisons might be marginally faster.
    uint32_t hash = util::string_hash(name);

    for( ; i != j; ++i)
    {
        uint32_t comp = util::string_hash((*i)->name);
        if(comp == hash) return (*i);
    }

    return nullptr;
}

int zXMLParser::ParseTree(const char** str, const int index,
                          util::XMLNode* parent)
{
    int i = index;

    while(str != nullptr && str[i] != nullptr)
    {
        // CreateNode() is responsible for increasing the list index.
        i = this->CreateNode(string_t(str[i]), parent, i,
                [str, this](const int i, XMLNode* pNode) -> int {
                    return this->ParseTree(str, i, pNode);
            }
        );

        // Some error occurred here, so we move back up
        // through the recursion stack.
        if(i < 0) return i;
    }

    return -1;
}

int zXMLParser::ParseTree(std::ifstream& file, const int index,
                          util::XMLNode* parent)
{
    string_t line;
    int i = index;

    while(std::getline(file, line))
    {
        i = this->CreateNode(line, parent, i,
                [&file, this](const int idex, XMLNode* pNode) -> int {
                    return this->ParseTree(file, idex, pNode);
            }
        );

        if(i == 0)
        {
            m_XMLTree.push_back(parent);
            i++;
        }
        else if(i < 0) break;
    }

    return i;
}

int zXMLParser::CreateNode(const string_t& line,
                           util::XMLNode* parent, int i,
                           std::function<int(const size_t,
                                             util::XMLNode*)> callback)
{
    // Check for blank lines and comments.
    if(line.empty() || (line.find("<!-- ") != string_t::npos &&
                        line.find(" -->")  != string_t::npos))
    {
        // Next line plz.
        return ++i;
    }

    // Some sort of start tag?
    else if(line.find('<') != string_t::npos)
    {
        // If there's a beginning bracket, make sure there's an end bracket.
        if(line.find('>') == string_t::npos)
        {
            this->ShowXMLError(i + 1, line, "missing closing bracket");
            return -1;
        }

        else if(line.find("/>") != string_t::npos)
        {
            this->ShowXMLError(i + 1, line, "single-line tags forbidden");
            return -1;
        }

        // If it's not an end tag, parse it and recurse on it.
        else if(line.find("</") == string_t::npos)
        {
            XMLNode* pNode  = new XMLNode;
            pNode->parent   = parent;
            pNode->children.clear();

            if(parent != nullptr) parent->children.push_back(pNode);

            // Parse the tag name (e.g. 'test' from <test>).
            const size_t index = line.find('<');
            const size_t end   = line.find(' ', index);
            const size_t end2  = line.find('>', index);

            // No closing tag or empty tag ('<>')
            if(end == end2 || end2 == index + 1)
            {
                ShowXMLError(i + 1, line, "invalid XML tag");
                delete pNode;
                return -1;
            }

            pNode->name = line.substr(index + 1, min(end, end2) - 1);

            util::stripl(pNode->name);
            util::stript(pNode->name);

            // Parse any options in the tag.
            if(end != string_t::npos &&
               line.find('=', index) != string_t::npos)
            {
                size_t last = end;

                for( ; ; )
                {
                    size_t start    = last + 1;
                    size_t keyindex = line.find('=', start);

                    // No more options?
                    if(keyindex == string_t::npos) break;

                    size_t valstart = line.find('"', keyindex);
                    size_t valend   = line.find('"', valstart + 1);

                    // Check if we have both characters.
                    // Offset the start so we skip the first " character in substr().
                    if(valstart++ == string_t::npos || valend == string_t::npos)
                    {
                        ShowXMLError(i + 1, line, "missing closing quote");

                        // Attempt to save by ending option at tag end.
                        valend = line.find('>', index);
                    }

                    string_t key = line.substr(start, keyindex - start);
                    string_t val = line.substr(valstart, valend - valstart);

                    util::stripl(key);
                    util::stript(key);

                    pNode->options[key] = val;
                    last = valend + 1;
                }
            }

            m_XMLTree.push_back(pNode);

            // Error so pass it along.
            if((i = callback(i + 1, pNode)) == -1)
                return i;

            // No error so next line.
            else return ++i;
        }

        // End of node
        else if(line.find("</") != string_t::npos)
        {
            // Next line plz.
            return ++i;
        }
    }

    // Non-XML line, so we just add it to the node's content.
    else
    {
        if(parent != nullptr) parent->content += line;
        return ++i;
    }

    return -1;
}
