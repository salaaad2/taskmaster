#include <iostream>
#include "Supervisor.hpp"
#include "Utils.hpp"

int main(int ac, char **av)
{
    string config_file, log_file;
    char * opt = NULL;
    bool help;

    help = false;
    if ((opt = Utils::GetCommandLineOption(ac, av, "--config-file")) != NULL)
    {config_file = opt;}
    if ((opt = Utils::GetCommandLineOption(ac, av, "--log-file")) != NULL)
    {log_file = opt;}
    if ((opt = Utils::GetCommandLineOption(ac, av, "--help")) != NULL)
    {help = true;}

    if (help)
    {return Utils::PrintHelp();}
    if (log_file.empty())
    {std::cout << "log file unspecified (--log-file), using default: ./taskmaster.log\n";}
    if (config_file.empty())
    {return Utils::MissingArgument("--config-file");}

    Supervisor s(config_file, log_file);
    if (!s.isConfigValid())
    {
        return (1);
    }
    else
    {
        s.init();
    }
    return 0;
}
