#include "Zenderer/Utilities/Helpers.hpp"

using namespace zen;

void util::stripl(string_t& text)
{
    if(!text.empty())
        text = text.substr(text.find_first_not_of(" \t"));
}

string_t util::stripl(const string_t& text)
{
    return !text.empty() ? text.substr(text.find_first_not_of(" \t")) : text;
}

void util::stript(string_t& text)
{
    text.resize(text.find_last_not_of(" \t"));
}

string_t util::stript(const string_t& text)
{
    string_t tmp = text;
    tmp.resize(tmp.find_last_not_of(" \t"));
    return text;
}

void util::strip(string_t& text)
{
    if(!text.empty())
        text = text.substr(text.find_first_not_of(" \t"),
                           text.find_last_not_of (" \t")+1);
}

string_t util::strip(const string_t& text)
{
    return !text.empty() ? text.substr(text.find_first_not_of(" \t"),
                                       text.find_last_not_of (" \t")+1)
                         : text;
}

std::vector<string_t> util::split(const string_t& text,
                                  const char delim,
                                  const size_t approx)
{
    std::vector<string_t> splitResults;
    if(approx) splitResults.resize(approx);

    auto i = text.begin(), j = text.end();
    auto last = i;
    for( ; i != j; )
    {
        if(*i == delim)
        {
            splitResults.emplace_back(last, i);
            last = ++i;
        }
        else
        {
            ++i;
        }
    }
    splitResults.emplace_back(last, j);
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
