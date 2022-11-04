#include <iostream>
#include <vector>

using std::string;

namespace Utils {
    void PrintError(
        const string & source,
        const string & reason);
    void PrintSuccess(
        const string & source,
        const string & reason);
    std::vector<std::string> SplitString(
        std::string source,
        const std::string &separator);

    /*
    ** with source a container with begin and end(),
    ** return a const char * vector to be used by
    ** C functions which require char*const*
     */
    template <typename T>
    std::vector<const char*> ContainerToConstChar(
            const std::string & name,
            const T & source)
    {
        std::vector<const char*> out;

        out.push_back(name.c_str());
        for (auto it = source.begin(); it != source.end(); ++it)
        {
            out.push_back((*it).c_str());
        }
        out.push_back(NULL);
        return out;
    }


    template <typename T>
    std::string JoinStrings(
            const T & source,
            const std::string & separator)
    {
        std::string out;
        auto it = source.begin();
        out.append(*it);
        ++it;
        for (; it != source.end(); ++it)
        {
            out.append(separator);
            out.append(*it);
        }
        return out;
    }
};
