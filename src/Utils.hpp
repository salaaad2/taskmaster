#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

using std::string;

#define IGNORE(x) (void)x;

namespace Utils {

    void LogError(
        std::fstream & stream,
        const string & source,
        const string & reason);
    void LogSuccess(
        std::fstream & stream,
        const string & source,
        const string & reason);
    void LogStatus(
        std::fstream & stream,
        const string & custom);

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

    char * GetCommandLineOption(int ac, char *av[], const string &option_flag);
    int PrintHelp();
    int MissingArgument(const string & argument);


    /*
    ** signal handler
    */
    void SignalLambdaWrapper(int signal);
};
