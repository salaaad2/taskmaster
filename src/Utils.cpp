#include "Utils.hpp"
#include <mutex>
#include <string>
#include <fstream>

namespace Utils {

void Log(
    std::fstream & stream,
    const string & type,
    const string & source,
    const string & reason)
{
    string s;
    s = "[" + std::to_string(std::time(nullptr)) + "] "+
        "taskmaster: " + type +
        ": " + source +
        ": " + reason + "\n";
    stream << s;
    stream.flush();
}

void LogSuccess(
    std::fstream & stream,
    const string & source,
    const string & reason)
{
    write_mutex.lock();
    Log(stream, "SUCCESS", source, reason);
    write_mutex.unlock();
}

void LogError(
    std::fstream & stream,
    const string & source,
    const string & reason)
{
    write_mutex.lock();
    Log(stream, "ERROR", source, reason);
    write_mutex.unlock();
}


void LogStatus(
    std::fstream & stream,
    const string & custom)
{
    write_mutex.lock();
    stream << custom;
    stream.flush();
    write_mutex.unlock();
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

char * GetCommandLineOption(int ac, char *av[], const string &option_flag)
{
    for (int i = 0; i < ac; ++i)
    {
        if (std::string(av[i]) == option_flag)
        {
            return av[i + 1];
        }
    }
    return NULL;
}

int PrintHelp()
{
    string out =
        "Usage\n  taskmaster [options]\n\nOptions:\n";
    out += "  --help\tprint this help\n";
    out += "  --config-file <path>\tpath to the config file (YAML)\n";
    out += "  --log-file <path>\tpath to the output log file\n";
    std::cout << out;
    return (0);
}

int MissingArgument(const string & argument)
{
    string out =
        "Missing argument: " + argument + "\n";
    std::cout << out;
    return (1);
}

}
