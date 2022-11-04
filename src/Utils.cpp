#include "Utils.hpp"

namespace Utils {

void PrintError(const string & source, const string & reason)
{
    std::cout << "taskmaster: error: " << source << ": " << reason << std::endl;
}

void PrintSuccess(const string & source, const string & reason)
{
    std::cout << "taskmaster: success: " << source << ": " << reason << std::endl;
}

/*
** return string vector split using
** separator
 */
std::vector<std::string> SplitString(
    std::string source,
    const std::string &separator)
{
    std::vector<std::string> out;
    int last_separator_pos = 0;

    while (source.find(separator) != std::string::npos)
    {
        out.push_back(source.substr(last_separator_pos, source.find(separator)));
        source.erase(0, source.find(separator) + separator.length());
    }
    if (source.size() != 0)
    {
        out.push_back(source);
    }
    return out;
}

}
