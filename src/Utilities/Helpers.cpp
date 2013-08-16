#include "Zenderer/Utilities/Helpers.hpp"

using namespace zen;

void util::stripl(string_t& text)
{
    text = text.substr(text.find_first_not_of(" \t"));
}

string_t util::stripl(const string_t& text)
{
    return text.substr(text.find_first_not_of(" \t"));
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

void strip(string_t& text)
{
    text = text.substr(text.find_first_not_of(" \t"),
                       text.find_last_not_of (" \t"));
}

string_t strip(const string_t& text)
{
    return text.substr(text.find_first_not_of(" \t"),
                       text.find_last_not_of (" \t"));
}

std::vector<string_t> util::split(const string_t& text,
                                  const char delim,
                                  const size_t approx)
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
            splitResults.emplace_back(text.substr(index, next - index));
        }
        else
        {
            // Add everything that's left.
            splitResults.emplace_back(text.substr(last, index));
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
