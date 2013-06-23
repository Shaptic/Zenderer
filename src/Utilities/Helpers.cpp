#include "Zenderer/Utilities/Helpers.hpp"

using namespace zen;

void util::stripl(std::string& text)
{
    size_t i = 0;    
    while(text[i++] == ' ' || text[i] == '\t' && i < text.length());
    text = text.substr(i - 1);
}

std::string util::stripl(const std::string& text)
{
    size_t i = 0;    
    while(text[i++] == ' ' || text[i] == '\t' && i < text.length());
    return text.substr(i - 1);
}

void util::stript(std::string& text)
{
    int i = text.length();    
    while(text[--i] == ' ' || text[i] == '\t' && i >= 0);
    text = text.substr(0, i + 1);
}

std::string util::stript(const std::string& text)
{
    int i = text.length();    
    while(text[--i] == ' ' || text[i] == '\t' && i >= 0);
    return text.substr(0, i + 1);
}

std::vector<string_t> util::split(const string_t& text, const char delim, const size_t approx)
{
    std::vector<string_t> splitResults;
    size_t last = 0;
    
    if(approx) splitResults.resize(approx);
    
    for( ; ; )
    {
        size_t index = text.find(delim, last);
        size_t next  = index - 1;

        if(index != string_t::npos)
        {
            // Skip block of delimiters.
            while(text[++index] == delim && index < text.length());

            // Block till end? Done.
            if(index >= text.length()) break;

            // Find next one.
            next = text.find(delim, index + 1);

            // Add and repeat.
            splitResults.push_back(text.substr(index, next - index));
        }
        else
        {
            // Add everything that's left.
            splitResults.push_back(text.substr(last, index));
            break;
        }

        // Start further in the string.
        last = next;
    }
    
    ZEN_ASSERT(!splitResults.empty());
    
    return splitResults;
}

string_t util::toupper(const string_t& str)
{
    string_t result(str);
    for(auto& i : result)
        i = ::toupper(i);
    
    return result;
}

void util::toupper(string_t& str)
{
    for(auto& i : str)
        i = ::toupper(i);
}
